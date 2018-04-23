import os
import sys
joinp = os.path.join
sys.path.insert(0, 'whitgl')
sys.path.insert(0, joinp('whitgl', 'input'))
import build

sys.path.insert(0, 'input')
import ninja_syntax


def do_game(name, extra_cflags, data_types):
  target = name.lower()
  srcdir = 'src'
  inputdir = joinp('whitgl', 'input')
  builddir = 'build'
  targetdir = joinp(builddir, 'out')
  if build.plat == 'Darwin':
    packagedir = joinp(targetdir, '%s.app' % name, 'Contents')
    executabledir = joinp(packagedir, 'MacOS')
    data_out = joinp(packagedir, 'Resources', 'data')
  else:
    executabledir = targetdir
    data_out = joinp(targetdir, 'data')
  objdir = joinp(builddir, 'obj')
  libdir = joinp(builddir, 'lib')
  data_in =  'data'
  if not os.path.exists('build'):
    os.makedirs('build')
  buildfile = open(joinp('build', 'build.ninja'), 'w')
  n = ninja_syntax.Writer(buildfile)
  n.variable('builddir', builddir)
  n.variable('scriptsdir', joinp('whitgl','scripts'))
  n.newline()
  cflags, ldflags = build.flags(inputdir)
  cflags = cflags + ' -Iwhitgl/inc -Isrc ' + extra_cflags
  if build.plat == 'Windows':
  	ldflags += " -lole32 -luuid"
  n.variable('cflags', cflags)
  n.variable('ldflags', ldflags)
  n.newline()
  build.rules(n)

  n.rule('rc',
    command='windres $in -O coff -o $out',
    description='WINDRES $in $out')
  obj = build.walk_src(n, srcdir, objdir)
  obj += build.walk_src(n, joinp('input', 'gif_lib'), objdir)
  if build.plat == 'Darwin':
    obj += n.build(joinp(objdir, 'nfd_cocoa.o'), 'cxx', joinp('input', 'nativefiledialog', 'src', 'nfd_cocoa.m'))
  else:
    obj += n.build(joinp(objdir, 'nfd_win.o'), 'cxx', joinp('input', 'nativefiledialog', 'src', 'nfd_win.cpp'))
  obj += n.build(joinp(objdir, 'nfd_common.o'), 'cxx', joinp('input', 'nativefiledialog', 'src', 'nfd_common.c'))
  whitgl = [joinp('whitgl','build','lib','whitgl.a')]
  ico = []
  if build.plat == 'Windows':
    ico += n.build(joinp(builddir, 'ico', '%s.res' % target), 'rc', joinp('windows', '%s.rc' % target))
  targets = []
  targets += n.build(joinp(executabledir, target), 'link', obj+whitgl+ico)
  n.newline()

  data = build.walk_data(n, data_in, data_out, data_types)

  targets += n.build('data', 'phony', data)
  n.newline()

  targets += build.copy_libs(n, inputdir, executabledir)

  if build.plat == 'Darwin':
    targets += n.build(joinp(packagedir, 'Info.plist'), 'cp', joinp(data_in, 'osx', 'Info.plist'))
    targets += n.build(joinp(packagedir, 'Resources', 'Icon.icns'), 'icon', joinp('art', 'icon', 'icon.png'))

  if build.plat == 'Windows':
    targets += n.build(joinp(executabledir, 'libgcc_s_dw2-1.dll'), 'cp', joinp('windows', 'libgcc_s_dw2-1.dll'))
    targets += n.build(joinp(executabledir, 'libwinpthread-1.dll'), 'cp', joinp('windows', 'libwinpthread-1.dll'))
    targets += n.build(joinp(executabledir, 'libstdc++-6.dll'), 'cp', joinp('windows', 'libstdc++-6.dll'))

  n.build('all', 'phony', targets)
  n.default('all')

do_game('Lofoten', '-Iinput -Iinput/nativefiledialog/src/include', ['png','ogg','obj'])
