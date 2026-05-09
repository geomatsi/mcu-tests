project('stm8 tests', 'c',
	version : '0.1',
)

stm8cc = find_program('sdcc', required : true)
stm8flash = find_program('stm8flash', required : false)
openocd = find_program('openocd', required : false)

if not (stm8flash.found() or openocd.found())
	error('Need either stm8flash or openocd to flash MCU...')
endif

stm8cc_args = ['-mstm8', '--Werror', '--std-c99']
stm8ld_args = ['-mstm8', '--out-fmt-ihx']
stm8cc_incs = ['-I' , '../include']

progs = [
	['blink.ihx', ['src/blink.c', 'src/delay.c'], 'stm8s103f3', 'GPIO/LED example'],
]

foreach p : progs

	gen = generator(stm8cc,
		output : '@BASENAME@.rel',
		arguments : stm8cc_args + stm8cc_incs + ['-c', '@INPUT@'] + ['-o', '@OUTPUT@'],
	)

	obj = gen.process(p[1])

	exe = custom_target(p[0],
		input : obj,
		output : p[0],
		command : [stm8cc, stm8ld_args, '@INPUT@', '-o', '@OUTPUT@'],
	)

	flash_stm8 = run_target('stm8flash_@0@'.format(p[0]),
		command : [stm8flash, '-s', 'flash', '-c', 'stlinkv2', '-p', p[2], '-w', '@0@'.format(exe.full_path())],
		depends : exe,
	)

	flash_ocd = run_target('openocd_@0@'.format(p[0]),
		command : [openocd, '-f', 'openocd/@0@.cfg'.format(p[2]), '-c', 'program @0@'.format(exe.full_path())],
		depends : exe,
	)

	debug = run_target('debug_@0@'.format(p[0]),
		command : [openocd, '-f', 'openocd/@0@.cfg'.format(p[2]), '-c', 'debug ()'],
		depends : exe,
	)
endforeach
