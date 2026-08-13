SECTIONS
{
	basebounds_section_mut_16 0x80000000000 : AT(0x80000000000)
	{
		KEEP(*(basebounds_section_mut_16))
	}
	basebounds_section_mut_16_end = 0x80000000000 + ALIGN(SIZEOF(basebounds_section_mut_16), 4096);
	basebounds_section_const_16 basebounds_section_mut_16_end : AT(basebounds_section_mut_16_end)
	{
		KEEP(*(basebounds_section_const_16))
	}
	basebounds_section_const_16_end = .;
	basebounds_section_mut_32 0xa0000000000 : AT(0xa0000000000)
	{
		KEEP(*(basebounds_section_mut_32))
	}
	basebounds_section_mut_32_end = 0xa0000000000 + ALIGN(SIZEOF(basebounds_section_mut_32), 4096);
	basebounds_section_const_32 basebounds_section_mut_32_end : AT(basebounds_section_mut_32_end)
	{
		KEEP(*(basebounds_section_const_32))
	}
	basebounds_section_const_32_end = .;
	basebounds_section_mut_64 0xc0000000000 : AT(0xc0000000000)
	{
		KEEP(*(basebounds_section_mut_64))
	}
	basebounds_section_mut_64_end = 0xc0000000000 + ALIGN(SIZEOF(basebounds_section_mut_64), 4096);
	basebounds_section_const_64 basebounds_section_mut_64_end : AT(basebounds_section_mut_64_end)
	{
		KEEP(*(basebounds_section_const_64))
	}
	basebounds_section_const_64_end = .;
	basebounds_section_mut_128 0xe0000000000 : AT(0xe0000000000)
	{
		KEEP(*(basebounds_section_mut_128))
	}
	basebounds_section_mut_128_end = 0xe0000000000 + ALIGN(SIZEOF(basebounds_section_mut_128), 4096);
	basebounds_section_const_128 basebounds_section_mut_128_end : AT(basebounds_section_mut_128_end)
	{
		KEEP(*(basebounds_section_const_128))
	}
	basebounds_section_const_128_end = .;
	basebounds_section_mut_256 0x100000000000 : AT(0x100000000000)
	{
		KEEP(*(basebounds_section_mut_256))
	}
	basebounds_section_mut_256_end = 0x100000000000 + ALIGN(SIZEOF(basebounds_section_mut_256), 4096);
	basebounds_section_const_256 basebounds_section_mut_256_end : AT(basebounds_section_mut_256_end)
	{
		KEEP(*(basebounds_section_const_256))
	}
	basebounds_section_const_256_end = .;
	basebounds_section_mut_512 0x120000000000 : AT(0x120000000000)
	{
		KEEP(*(basebounds_section_mut_512))
	}
	basebounds_section_mut_512_end = 0x120000000000 + ALIGN(SIZEOF(basebounds_section_mut_512), 4096);
	basebounds_section_const_512 basebounds_section_mut_512_end : AT(basebounds_section_mut_512_end)
	{
		KEEP(*(basebounds_section_const_512))
	}
	basebounds_section_const_512_end = .;
	basebounds_section_mut_1024 0x140000000000 : AT(0x140000000000)
	{
		KEEP(*(basebounds_section_mut_1024))
	}
	basebounds_section_mut_1024_end = 0x140000000000 + ALIGN(SIZEOF(basebounds_section_mut_1024), 4096);
	basebounds_section_const_1024 basebounds_section_mut_1024_end : AT(basebounds_section_mut_1024_end)
	{
		KEEP(*(basebounds_section_const_1024))
	}
	basebounds_section_const_1024_end = .;
	basebounds_section_mut_2048 0x160000000000 : AT(0x160000000000)
	{
		KEEP(*(basebounds_section_mut_2048))
	}
	basebounds_section_mut_2048_end = 0x160000000000 + ALIGN(SIZEOF(basebounds_section_mut_2048), 4096);
	basebounds_section_const_2048 basebounds_section_mut_2048_end : AT(basebounds_section_mut_2048_end)
	{
		KEEP(*(basebounds_section_const_2048))
	}
	basebounds_section_const_2048_end = .;
	basebounds_section_mut_4096 0x180000000000 : AT(0x180000000000)
	{
		KEEP(*(basebounds_section_mut_4096))
	}
	basebounds_section_mut_4096_end = 0x180000000000 + ALIGN(SIZEOF(basebounds_section_mut_4096), 4096);
	basebounds_section_const_4096 basebounds_section_mut_4096_end : AT(basebounds_section_mut_4096_end)
	{
		KEEP(*(basebounds_section_const_4096))
	}
	basebounds_section_const_4096_end = .;
	basebounds_section_mut_8192 0x1a0000000000 : AT(0x1a0000000000)
	{
		KEEP(*(basebounds_section_mut_8192))
	}
	basebounds_section_mut_8192_end = 0x1a0000000000 + ALIGN(SIZEOF(basebounds_section_mut_8192), 8192);
	basebounds_section_const_8192 basebounds_section_mut_8192_end : AT(basebounds_section_mut_8192_end)
	{
		KEEP(*(basebounds_section_const_8192))
	}
	basebounds_section_const_8192_end = .;
	basebounds_section_mut_16384 0x1c0000000000 : AT(0x1c0000000000)
	{
		KEEP(*(basebounds_section_mut_16384))
	}
	basebounds_section_mut_16384_end = 0x1c0000000000 + ALIGN(SIZEOF(basebounds_section_mut_16384), 16384);
	basebounds_section_const_16384 basebounds_section_mut_16384_end : AT(basebounds_section_mut_16384_end)
	{
		KEEP(*(basebounds_section_const_16384))
	}
	basebounds_section_const_16384_end = .;
	basebounds_section_mut_32768 0x1e0000000000 : AT(0x1e0000000000)
	{
		KEEP(*(basebounds_section_mut_32768))
	}
	basebounds_section_mut_32768_end = 0x1e0000000000 + ALIGN(SIZEOF(basebounds_section_mut_32768), 32768);
	basebounds_section_const_32768 basebounds_section_mut_32768_end : AT(basebounds_section_mut_32768_end)
	{
		KEEP(*(basebounds_section_const_32768))
	}
	basebounds_section_const_32768_end = .;
	basebounds_section_mut_65536 0x200000000000 : AT(0x200000000000)
	{
		KEEP(*(basebounds_section_mut_65536))
	}
	basebounds_section_mut_65536_end = 0x200000000000 + ALIGN(SIZEOF(basebounds_section_mut_65536), 65536);
	basebounds_section_const_65536 basebounds_section_mut_65536_end : AT(basebounds_section_mut_65536_end)
	{
		KEEP(*(basebounds_section_const_65536))
	}
	basebounds_section_const_65536_end = .;
	basebounds_section_mut_131072 0x220000000000 : AT(0x220000000000)
	{
		KEEP(*(basebounds_section_mut_131072))
	}
	basebounds_section_mut_131072_end = 0x220000000000 + ALIGN(SIZEOF(basebounds_section_mut_131072), 131072);
	basebounds_section_const_131072 basebounds_section_mut_131072_end : AT(basebounds_section_mut_131072_end)
	{
		KEEP(*(basebounds_section_const_131072))
	}
	basebounds_section_const_131072_end = .;
	basebounds_section_mut_262144 0x240000000000 : AT(0x240000000000)
	{
		KEEP(*(basebounds_section_mut_262144))
	}
	basebounds_section_mut_262144_end = 0x240000000000 + ALIGN(SIZEOF(basebounds_section_mut_262144), 262144);
	basebounds_section_const_262144 basebounds_section_mut_262144_end : AT(basebounds_section_mut_262144_end)
	{
		KEEP(*(basebounds_section_const_262144))
	}
	basebounds_section_const_262144_end = .;
	basebounds_section_mut_524288 0x260000000000 : AT(0x260000000000)
	{
		KEEP(*(basebounds_section_mut_524288))
	}
	basebounds_section_mut_524288_end = 0x260000000000 + ALIGN(SIZEOF(basebounds_section_mut_524288), 524288);
	basebounds_section_const_524288 basebounds_section_mut_524288_end : AT(basebounds_section_mut_524288_end)
	{
		KEEP(*(basebounds_section_const_524288))
	}
	basebounds_section_const_524288_end = .;
	basebounds_section_mut_1048576 0x280000000000 : AT(0x280000000000)
	{
		KEEP(*(basebounds_section_mut_1048576))
	}
	basebounds_section_mut_1048576_end = 0x280000000000 + ALIGN(SIZEOF(basebounds_section_mut_1048576), 1048576);
	basebounds_section_const_1048576 basebounds_section_mut_1048576_end : AT(basebounds_section_mut_1048576_end)
	{
		KEEP(*(basebounds_section_const_1048576))
	}
	basebounds_section_const_1048576_end = .;
	basebounds_section_mut_2097152 0x2a0000000000 : AT(0x2a0000000000)
	{
		KEEP(*(basebounds_section_mut_2097152))
	}
	basebounds_section_mut_2097152_end = 0x2a0000000000 + ALIGN(SIZEOF(basebounds_section_mut_2097152), 2097152);
	basebounds_section_const_2097152 basebounds_section_mut_2097152_end : AT(basebounds_section_mut_2097152_end)
	{
		KEEP(*(basebounds_section_const_2097152))
	}
	basebounds_section_const_2097152_end = .;
	basebounds_section_mut_4194304 0x2c0000000000 : AT(0x2c0000000000)
	{
		KEEP(*(basebounds_section_mut_4194304))
	}
	basebounds_section_mut_4194304_end = 0x2c0000000000 + ALIGN(SIZEOF(basebounds_section_mut_4194304), 4194304);
	basebounds_section_const_4194304 basebounds_section_mut_4194304_end : AT(basebounds_section_mut_4194304_end)
	{
		KEEP(*(basebounds_section_const_4194304))
	}
	basebounds_section_const_4194304_end = .;
	basebounds_section_mut_8388608 0x2e0000000000 : AT(0x2e0000000000)
	{
		KEEP(*(basebounds_section_mut_8388608))
	}
	basebounds_section_mut_8388608_end = 0x2e0000000000 + ALIGN(SIZEOF(basebounds_section_mut_8388608), 8388608);
	basebounds_section_const_8388608 basebounds_section_mut_8388608_end : AT(basebounds_section_mut_8388608_end)
	{
		KEEP(*(basebounds_section_const_8388608))
	}
	basebounds_section_const_8388608_end = .;
	basebounds_section_mut_16777216 0x300000000000 : AT(0x300000000000)
	{
		KEEP(*(basebounds_section_mut_16777216))
	}
	basebounds_section_mut_16777216_end = 0x300000000000 + ALIGN(SIZEOF(basebounds_section_mut_16777216), 16777216);
	basebounds_section_const_16777216 basebounds_section_mut_16777216_end : AT(basebounds_section_mut_16777216_end)
	{
		KEEP(*(basebounds_section_const_16777216))
	}
	basebounds_section_const_16777216_end = .;
	basebounds_section_mut_33554432 0x320000000000 : AT(0x320000000000)
	{
		KEEP(*(basebounds_section_mut_33554432))
	}
	basebounds_section_mut_33554432_end = 0x320000000000 + ALIGN(SIZEOF(basebounds_section_mut_33554432), 33554432);
	basebounds_section_const_33554432 basebounds_section_mut_33554432_end : AT(basebounds_section_mut_33554432_end)
	{
		KEEP(*(basebounds_section_const_33554432))
	}
	basebounds_section_const_33554432_end = .;
	basebounds_section_mut_67108864 0x340000000000 : AT(0x340000000000)
	{
		KEEP(*(basebounds_section_mut_67108864))
	}
	basebounds_section_mut_67108864_end = 0x340000000000 + ALIGN(SIZEOF(basebounds_section_mut_67108864), 67108864);
	basebounds_section_const_67108864 basebounds_section_mut_67108864_end : AT(basebounds_section_mut_67108864_end)
	{
		KEEP(*(basebounds_section_const_67108864))
	}
	basebounds_section_const_67108864_end = .;
	basebounds_section_mut_134217728 0x360000000000 : AT(0x360000000000)
	{
		KEEP(*(basebounds_section_mut_134217728))
	}
	basebounds_section_mut_134217728_end = 0x360000000000 + ALIGN(SIZEOF(basebounds_section_mut_134217728), 134217728);
	basebounds_section_const_134217728 basebounds_section_mut_134217728_end : AT(basebounds_section_mut_134217728_end)
	{
		KEEP(*(basebounds_section_const_134217728))
	}
	basebounds_section_const_134217728_end = .;
	basebounds_section_mut_268435456 0x380000000000 : AT(0x380000000000)
	{
		KEEP(*(basebounds_section_mut_268435456))
	}
	basebounds_section_mut_268435456_end = 0x380000000000 + ALIGN(SIZEOF(basebounds_section_mut_268435456), 268435456);
	basebounds_section_const_268435456 basebounds_section_mut_268435456_end : AT(basebounds_section_mut_268435456_end)
	{
		KEEP(*(basebounds_section_const_268435456))
	}
	basebounds_section_const_268435456_end = .;
	basebounds_section_mut_536870912 0x3a0000000000 : AT(0x3a0000000000)
	{
		KEEP(*(basebounds_section_mut_536870912))
	}
	basebounds_section_mut_536870912_end = 0x3a0000000000 + ALIGN(SIZEOF(basebounds_section_mut_536870912), 536870912);
	basebounds_section_const_536870912 basebounds_section_mut_536870912_end : AT(basebounds_section_mut_536870912_end)
	{
		KEEP(*(basebounds_section_const_536870912))
	}
	basebounds_section_const_536870912_end = .;
	basebounds_section_mut_1073741824 0x3c0000000000 : AT(0x3c0000000000)
	{
		KEEP(*(basebounds_section_mut_1073741824))
	}
	basebounds_section_mut_1073741824_end = 0x3c0000000000 + ALIGN(SIZEOF(basebounds_section_mut_1073741824), 1073741824);
	basebounds_section_const_1073741824 basebounds_section_mut_1073741824_end : AT(basebounds_section_mut_1073741824_end)
	{
		KEEP(*(basebounds_section_const_1073741824))
	}
	basebounds_section_const_1073741824_end = .;
	basebounds_section_mut_2147483648 0x3e0000000000 : AT(0x3e0000000000)
	{
		KEEP(*(basebounds_section_mut_2147483648))
	}
	basebounds_section_mut_2147483648_end = 0x3e0000000000 + ALIGN(SIZEOF(basebounds_section_mut_2147483648), 2147483648);
	basebounds_section_const_2147483648 basebounds_section_mut_2147483648_end : AT(basebounds_section_mut_2147483648_end)
	{
		KEEP(*(basebounds_section_const_2147483648))
	}
	basebounds_section_const_2147483648_end = .;
	basebounds_section_mut_4294967296 0x400000000000 : AT(0x400000000000)
	{
		KEEP(*(basebounds_section_mut_4294967296))
	}
	basebounds_section_mut_4294967296_end = 0x400000000000 + ALIGN(SIZEOF(basebounds_section_mut_4294967296), 4294967296);
	basebounds_section_const_4294967296 basebounds_section_mut_4294967296_end : AT(basebounds_section_mut_4294967296_end)
	{
		KEEP(*(basebounds_section_const_4294967296))
	}
	basebounds_section_const_4294967296_end = .;
	basebounds_section_mut_8589934592 0x420000000000 : AT(0x420000000000)
	{
		KEEP(*(basebounds_section_mut_8589934592))
	}
	basebounds_section_mut_8589934592_end = 0x420000000000 + ALIGN(SIZEOF(basebounds_section_mut_8589934592), 8589934592);
	basebounds_section_const_8589934592 basebounds_section_mut_8589934592_end : AT(basebounds_section_mut_8589934592_end)
	{
		KEEP(*(basebounds_section_const_8589934592))
	}
	basebounds_section_const_8589934592_end = .;
	basebounds_section_mut_17179869184 0x440000000000 : AT(0x440000000000)
	{
		KEEP(*(basebounds_section_mut_17179869184))
	}
	basebounds_section_mut_17179869184_end = 0x440000000000 + ALIGN(SIZEOF(basebounds_section_mut_17179869184), 17179869184);
	basebounds_section_const_17179869184 basebounds_section_mut_17179869184_end : AT(basebounds_section_mut_17179869184_end)
	{
		KEEP(*(basebounds_section_const_17179869184))
	}
	basebounds_section_const_17179869184_end = .;
	basebounds_section_mut_34359738368 0x460000000000 : AT(0x460000000000)
	{
		KEEP(*(basebounds_section_mut_34359738368))
	}
	basebounds_section_mut_34359738368_end = 0x460000000000 + ALIGN(SIZEOF(basebounds_section_mut_34359738368), 34359738368);
	basebounds_section_const_34359738368 basebounds_section_mut_34359738368_end : AT(basebounds_section_mut_34359738368_end)
	{
		KEEP(*(basebounds_section_const_34359738368))
	}
	basebounds_section_const_34359738368_end = .;
	basebounds_section_mut_68719476736 0x480000000000 : AT(0x480000000000)
	{
		KEEP(*(basebounds_section_mut_68719476736))
	}
	basebounds_section_mut_68719476736_end = 0x480000000000 + ALIGN(SIZEOF(basebounds_section_mut_68719476736), 68719476736);
	basebounds_section_const_68719476736 basebounds_section_mut_68719476736_end : AT(basebounds_section_mut_68719476736_end)
	{
		KEEP(*(basebounds_section_const_68719476736))
	}
	basebounds_section_const_68719476736_end = .;
	basebounds_section_mut_137438953472 0x4a0000000000 : AT(0x4a0000000000)
	{
		KEEP(*(basebounds_section_mut_137438953472))
	}
	basebounds_section_mut_137438953472_end = 0x4a0000000000 + ALIGN(SIZEOF(basebounds_section_mut_137438953472), 137438953472);
	basebounds_section_const_137438953472 basebounds_section_mut_137438953472_end : AT(basebounds_section_mut_137438953472_end)
	{
		KEEP(*(basebounds_section_const_137438953472))
	}
	basebounds_section_const_137438953472_end = .;
	basebounds_section_mut_274877906944 0x4c0000000000 : AT(0x4c0000000000)
	{
		KEEP(*(basebounds_section_mut_274877906944))
	}
	basebounds_section_mut_274877906944_end = 0x4c0000000000 + ALIGN(SIZEOF(basebounds_section_mut_274877906944), 274877906944);
	basebounds_section_const_274877906944 basebounds_section_mut_274877906944_end : AT(basebounds_section_mut_274877906944_end)
	{
		KEEP(*(basebounds_section_const_274877906944))
	}
	basebounds_section_const_274877906944_end = .;
	basebounds_section_mut_549755813888 0x4e0000000000 : AT(0x4e0000000000)
	{
		KEEP(*(basebounds_section_mut_549755813888))
	}
	basebounds_section_mut_549755813888_end = 0x4e0000000000 + ALIGN(SIZEOF(basebounds_section_mut_549755813888), 549755813888);
	basebounds_section_const_549755813888 basebounds_section_mut_549755813888_end : AT(basebounds_section_mut_549755813888_end)
	{
		KEEP(*(basebounds_section_const_549755813888))
	}
	basebounds_section_const_549755813888_end = .;
	basebounds_section_mut_1099511627776 0x500000000000 : AT(0x500000000000)
	{
		KEEP(*(basebounds_section_mut_1099511627776))
	}
	basebounds_section_mut_1099511627776_end = 0x500000000000 + ALIGN(SIZEOF(basebounds_section_mut_1099511627776), 1099511627776);
	basebounds_section_const_1099511627776 basebounds_section_mut_1099511627776_end : AT(basebounds_section_mut_1099511627776_end)
	{
		KEEP(*(basebounds_section_const_1099511627776))
	}
	basebounds_section_const_1099511627776_end = .;
}
INSERT AFTER .strtab;
