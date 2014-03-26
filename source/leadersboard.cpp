
#include <s3e.h>
#include "game_data.h"
#include "leadersboard.h"
#include "dgreed/utils.h"
#include "dgreed/darray.h"

#define CACHE_FILE writePath("slcache.db")
#define CONFIG_INI writePath("config.ini")

typedef struct {
  int result, sec, rank;
  bool my;
  char name[SC_USERNAME_MAX+3];
  int name_sz;
} CachedScore;
DArray leaderboard_cache[GameNumDifficulties]; // cache leaderboards for offline access

static void _load_cache_array(DArray &cache, FileHandle f);
static void _load_cache();
static void _flush_cache();

void _request_callback(int mode, SCL_ScoreList scores) {
    // reload list:
    leaderboard_cache[mode].size = 0;
    const int lc = SCL_GetLeadersboardCount(scores);
    for (int r=0; r<lc; ++r) {
        CachedScore el;
        const char *user = SCL_GetLeadersboardUserAt(scores, r);
        const int user_sz = strlen(user);
        char user_disp[SC_USERNAME_MAX+1] = { 0 }; strncpy(user_disp, user, user_sz>(SC_USERNAME_MAX+2)?SC_USERNAME_MAX:user_sz); // disp. max SC_USERNAME_MAX chars of user name + ".." = SC_USERNAME_MAX+2
        sprintf(el.name, "%s%s",
                user_disp,
                user_sz>(SC_USERNAME_MAX+2)?"..":"");
        el.name_sz = strlen(el.name);
        // el.spaces = 0; char *s = user_disp; while (*s++) if(*s == ' ') el.spaces++;
        el.result = (int)SCL_GetLeadersboardResultAt(scores, r);
        el.my = SCL_IsMyLeadersboardAt(scores, r);
        
        darray_append(&leaderboard_cache[mode], &el);
    }
    _flush_cache();
}

static void _load_cache_array(DArray &cache, FileHandle f) {
    file_read(f, &cache, sizeof(cache));
    
    size_t sizeof_score = cache.item_size;
    
    if(sizeof(CachedScore) != sizeof_score) {
        fprintf(stderr, "[scoreloop] Score queues data size mismatch");
        cache = darray_create(sizeof(CachedScore), 0);
        return;
    }
    
    fprintf(stderr, "[scoreloop] Restored %d score requests\n", sizeof_score);
    
    size_t score_bytes = sizeof_score * cache.reserved;
    cache.data = MEM_ALLOC(score_bytes);
    file_read(f, cache.data, score_bytes);
}

static void _load_cache() {
    if(file_exists(CACHE_FILE)) {
        FileHandle f = file_open(CACHE_FILE);
        
        uint32 magic = file_read_uint32(f);
        if(magic != FOURCC('G', 'S', 'Q', '1')) {
            fprintf(stderr, "[scoreloop] Unable to load score queues");
            return;
        }
        
	for (int d=0; d<GameNumDifficulties; ++d)
	  _load_cache_array(leaderboard_cache[d], f);
        
        file_close(f);
    } else {
      for (int d=0; d<GameNumDifficulties; ++d)
        leaderboard_cache[d] = darray_create(sizeof(CachedScore), 0);
    }
}

static void _flush_cache() {
    FileHandle f = file_create(CACHE_FILE);
    
    file_write_uint32(f, FOURCC('G', 'S', 'Q', '1'));
    for(int mode=0; mode<GameNumDifficulties; ++mode) {
        file_write(f, &leaderboard_cache[mode], sizeof(leaderboard_cache[mode]));
        file_write(f, leaderboard_cache[mode].data, sizeof(CachedScore) * leaderboard_cache[mode].reserved);
    }
    file_close(f);
}

// ==========
LeadersboardScore::LeadersboardScore(std::string _name, int _level, int _sec, bool _my) : 
  name(_name),
  level(_level),
  sec(_sec),
  my(_my) { }

// ==========
Leadersboard::Leadersboard() {
  _leadersboard = SCL_InitLeadersboard();
}

bool Leadersboard::isAvailable() {
  #ifdef __QNXNTO__
  return true;
  #else
  return false;
  #endif
}

void Leadersboard::load() {
  if (_leadersboard) {
    _load_cache(); // load cached data
    for (int d=0; d<GameNumDifficulties; ++d)
      SCL_SetLeadersboardDirty(d); // request to download
  }
}

void Leadersboard::update() {
  if (isAvailable()) {
    // request fresh status if hiscore has changed:
    if (SCL_GetLeadersboardStatus(GameData::getInstance()->difficulty) == SC_STATUS_DIRTY)
      if (SCL_GetLeadersboardLiveRequests() == 0)
	SCL_RequestLeadersboard(GameData::getInstance()->difficulty, _request_callback); // wait for all scores to be send before refreshing leaderboard
  }
}

LeadersboardScore Leadersboard::getLeadersboardScore(int p) {
  DArray &cache = leaderboard_cache[GameData::getInstance()->difficulty];
  if (p < cache.size) {
    CachedScore *score = (CachedScore*)darray_get(&cache, p);
    return LeadersboardScore(score->name, score->result, score->sec, score->my);
  }
  return LeadersboardScore("-/-", 0, 0, false);
}

LeadersboardScore Leadersboard::getMyLeadersboardScore() {
  DArray &cache = leaderboard_cache[GameData::getInstance()->difficulty];
  for (int r=0; r<cache.size; ++r) {
    CachedScore *score = (CachedScore*)darray_get(&cache, r);
    if (score->my) return LeadersboardScore(score->name, score->result, score->sec, score->my);
  }
  return LeadersboardScore("-/-", 0, 0, false);
}

Leadersboard* Leadersboard::instance = NULL;

Leadersboard* Leadersboard::getInstance() {
	if(instance == NULL)
		instance = new Leadersboard();
	return instance;
}
