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
  n.variable('cflags', cflags)
  n.variable('ldflags', ldflags)
  n.newline()
  build.rules(n)
  obj = build.walk_src(n, srcdir, objdir)
  obj += build.walk_src(n, joinp('input', 'gif_lib'), objdir)
  whitgl = [joinp('whitgl','build','lib','whitgl.a')]
  targets = []
  targets += n.build(joinp(executabledir, target), 'link', obj+whitgl)
  n.newline()

  data = build.walk_data(n, data_in, data_out, data_types)

  targets += n.build('data', 'phony', data)
  n.newline()

  targets += build.copy_libs(n, inputdir, executabledir)

  if build.plat == 'Darwin':
    targets += n.build(joinp(packagedir, 'Info.plist'), 'cp', joinp(data_in, 'osx', 'Info.plist'))
    targets += n.build(joinp(packagedir, 'Resources', 'Icon.icns'), 'icon', joinp('art', 'icon', 'icon.png'))

  n.build('all', 'phony', targets)
  n.default('all')

do_game('Lofoten', '-Iinput', ['png','ogg','obj'])
