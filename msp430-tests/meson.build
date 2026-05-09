project('msp430 tests', 'c',
	version : '0.1',
	default_options : [ 'buildtype=minsize', 'c_std=c99' ],
)

assert(meson.is_cross_build(), 'Use cross-build for msp430 target')

echo = find_program('echo', required : true)
mspdebug = find_program('mspdebug', required : true)

incdir = include_directories('include')

progs = [
	['blink', ['src/blink.c', 'src/delay.c'], 'GPIO/LED example', 'msp430g2231'],
	['button', ['src/button.c'], 'GPIO/button example', 'msp430g2231'],
]

t_mspflash = 'flash_@0@'
t_mspchip = '-mmcu=@0@'

foreach p : progs
	e = executable(p[0], p[1],
		c_args : t_mspchip.format(p[3]),
		link_args : t_mspchip.format(p[3]),
		include_directories : incdir,
		build_by_default : false,
	)

	r = run_target(t_mspflash.format(p[0]),
		command : [mspdebug, 'rf2500', 'prog @0@'.format(e.full_path())],
		depends : e,
	)

	message('@0@ (@1@) targets: @0@ @2@'.format(p[0], p[2], t_mspflash.format(p[0])))
endforeach
