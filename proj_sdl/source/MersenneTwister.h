#ifndef MERSENNETWISTER_H_INCLUDED
#define MERSENNETWISTER_H_INCLUDED


class MersenneTwister
{
public:

    // uses init_genrand to initialize the rng
    explicit MersenneTwister(unsigned long s = 5489UL);
    // uses init_by_array to initialize the rng
    explicit MersenneTwister(unsigned long init_key[], int key_length);
    ~MersenneTwister();

    /* initializes state[N] with a seed */
    void init_genrand(unsigned long s);

    /* initialize by an array with array-length */
    /* init_key is the array for initializing keys */
    /* key_length is its length */
    void init_by_array(unsigned long init_key[], int key_length);

    /* generates a random number on [0,0xffffffff]-interval */
    unsigned long genrand_int32();

    /* generates a random number on [0,0x7fffffff]-interval */
    long genrand_int31();

    /* generates a random number on [0,1]-real-interval */
    double genrand_real1();

    /* generates a random number on [0,1)-real-interval */
    double genrand_real2();

    /* generates a random number on (0,1)-real-interval */
    double genrand_real3();

    /* generates a random number on [0,1) with 53-bit resolution*/
    double genrand_res53();

private:

    enum MT_ENUM
    {
        N = 624
    };

    void next_state();

    unsigned long state[N]; /* the array for the state vector */
    int left;
    //int initf;
    unsigned long *next;
};


#endif // !MERSENNETWISTER_H_INCLUDED
