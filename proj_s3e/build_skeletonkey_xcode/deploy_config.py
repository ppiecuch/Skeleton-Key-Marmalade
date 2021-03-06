# -*- coding: utf-8 -*-
# Deployment settings for SkeletonKey.
# This file is autogenerated by the mkb system and used by the s3e deployment
# tool during the build process.

config = {}
cmdline = ['/Developer/Marmalade/6.4/s3e/makefile_builder/mkb.py', '--deploy=bb10', '--deployargs=--config=dist', '--deploy-only']
mkb = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/SkeletonKey.mkb'
mkf = ['/Developer/Marmalade/6.4/modules/iw2d/iw2d.mkf', '/Developer/Marmalade/6.4/modules/legacy/iw2d/iw2d_legacy.mkf', '/Developer/Marmalade/6.4/modules/iwgx/iwgx.mkf', '/Developer/Marmalade/6.4/modules/legacy/iwgx/iwgx_legacy.mkf', '/Developer/Marmalade/6.4/modules/iwgeom/iwgeom.mkf', '/Developer/Marmalade/6.4/modules/iwutil/iwutil.mkf', '/Developer/Marmalade/6.4/modules/third_party/libjpeg/libjpeg.mkf', '/Developer/Marmalade/6.4/modules/third_party/libpng/libpng.mkf', '/Developer/Marmalade/6.4/modules/third_party/zlib/zlib.mkf', '/Developer/Marmalade/6.4/modules/iwresmanager/iwresmanager.mkf', '/Developer/Marmalade/6.4/modules/legacy/iwresmanager/iwresmanager_legacy.mkf', '/Developer/Marmalade/6.4/modules/iwgl/iwgl.mkf', '/Developer/Marmalade/6.4/modules/iwgxfont/iwgxfont.mkf', '/Developer/Marmalade/6.4/modules/legacy/iwgxfont/iwgxfont_legacy.mkf', '/Developer/Marmalade/6.4/modules/third_party/tiniconv/tiniconv.mkf', '/Developer/Marmalade/6.4/modules/derbh/derbh.mkf', '/Developer/Marmalade/6.4/extensions/s3eIOSBackgroundMusic/s3eIOSBackgroundMusic.mkf']

class DeployConfig(object):
    pass

######### ASSET GROUPS #############

assets = {}

assets['Default'] = [
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/settings.db', 'settings.db', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/levels.db', 'levels.db', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/saved_game.db', 'saved_game.db', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/achievements.db', 'achievements.db', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/splash.jpg', 'splash.jpg', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/splash_bb10.jpg', 'splash_bb10.jpg', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/menu.mp3', 'menu.mp3', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/gameplay1.mp3', 'gameplay1.mp3', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/gameplay2.mp3', 'gameplay2.mp3', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/gameplay3.mp3', 'gameplay3.mp3', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/achievements.group', 'achievements.group', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/game_menu.group', 'game_menu.group', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/map.group', 'map.group', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/nag.group', 'nag.group', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/select_level.group', 'select_level.group', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/game.group', 'game.group', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/instructions.group', 'instructions.group', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/menu.group', 'menu.group', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/options.group', 'options.group', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/archive/sound.dz', 'sound.dz', 0),
    ('/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/archive/gles1.dz', 'gles1.dz', 0),
]

######### DEFAULT CONFIG #############

class DefaultConfig(DeployConfig):
    embed_icf = -1
    name = 'SkeletonKey'
    pub_sign_key = 0
    priv_sign_key = 0
    caption = 'SkeletonKey'
    long_caption = 'SkeletonKey'
    version = [1, 0, 1]
    config = ['/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data/app.icf']
    data_dir = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/data'
    iphone_link_lib = ['s3eIOSBackgroundMusic']
    linux_ext_lib = []
    copyright = '(c) 2012,2013 KomSoft Oprogramowanie'
    android_extra_application_manifest = []
    iphone_link_libdir = ['/Developer/Marmalade/6.4/extensions/s3eIOSBackgroundMusic/lib/iphone']
    iphone_link_libdirs = []
    blackberry_authorid = 'gYAAgJWWmDfer27CLcxm_P3mzK0'
    splashscreen = '/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/s3e/splash'
    iphone_link_opts = []
    provider = 'KomSoft'
    android_external_jars = []
    android_external_res = []
    android_supports_gl_texture = []
    android_extra_manifest = []
    icon = '/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/s3e/icons'
    win32_ext_dll = []
    android_so = []
    osx_ext_dll = []
    version_string = '1.0.1'
    blackberry_permissions_access_shared = 1
    blackberry_author = 'KomSoft Oprogramowanie'
    iphone_link_libs = []
    target = {
         'x86' : {
                   'debug'   : r'/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/build_skeletonkey_xcode/build/Debug/SkeletonKey.s86',
                   'release' : r'/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/build_skeletonkey_xcode/build/Release/SkeletonKey.s86',
                 },
         'arm_gcc' : {
                   'debug'   : r'/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/build_skeletonkey_xcode/build/Debug ARM/SkeletonKey.s3e',
                   'release' : r'/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/build_skeletonkey_xcode/build/Release ARM/SkeletonKey.s3e',
                 },
        }
    assets = assets['Default']

default = DefaultConfig()

######### Configuration: android

c = DeployConfig()
config['android'] = c
c.os = ['android']
c.target_folder = 'android'
c.android_pkgname = 'com.insurgentgames.skeletonkey'
c.android_icon = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/android/icon.png'
c.android_manifest = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/android/AndroidManifest.xml'
c.version_string = '1.0'

######### Configuration: dist

c = DeployConfig()
config['dist'] = c
c.target_folder = 'dist'
c.blackberry_cskpass = 'awec12'
c.blackberry_keystore = '/Users/pawelp/Library/Research In Motion/author.p12'
c.blackberry_storepass = 'awec12'

######### Configuration: iphone

c = DeployConfig()
config['iphone'] = c
c.os = ['iphone']
c.target_folder = 'iphone'
c.iphone_icon_high_res = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/ios/icon_114.png'
c.iphone_provisioning_profile = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/ios/SkeletonKeyAdHoc.mobileprovision'
c.iphone_prerendered_icon = 1
c.iphone_appid = 'com.insurgentgames.skeletonkey'
c.iphone_icon_ipad = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/ios/icon_72.png'
c.iphone_splash = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/ios/Default.png'
c.version_string = '1.7'
c.iphone_icon = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/ios/icon.png'
c.iphone_sign_for_distribution = 1
c.itunes_artwork = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/ios/iTunes_large.png'

######### Configuration: bada

c = DeployConfig()
config['bada'] = c
c.os = ['bada']
c.target_folder = 'bada'
c.bada_splash = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/bada/Splash.png'
c.version_string = '1.0.0'
c.bada_icon = '/Volumes/MacHDD/Users/pawelp/Private/Projekty/_OPEN_SOURCE_PROJECTS/Skeleton-Key-Marmalade/other_files/bada/Application.png'
