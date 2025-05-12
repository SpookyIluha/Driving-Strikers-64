#a list of custom asset flags to use when converting to Libradgon formats

filesystem/fonts/Kanit-Black.font64: 					MKFONT_FLAGS+= --size 18  --range 20-7F --range 80-FF --range 100-17F --range 180-24F
filesystem/fonts/Kanit-BlackItalic.font64: 				MKFONT_FLAGS+= --size 42  --range 20-7F --range 80-FF --range 100-17F --range 180-24F
filesystem/fonts/Kanit-BlackItalic_small.font64: 		MKFONT_FLAGS+= --size 18  --range 20-7F --range 80-FF --range 100-17F --range 180-24F
filesystem/fonts/LuckiestGuy-Regular.font64: 			MKFONT_FLAGS+= --size 26  
filesystem/fonts/RobotoCondensed-Bold.font64: 			MKFONT_FLAGS+= --size 24  
filesystem/fonts/UbuntuMono-Bold.font64: 				MKFONT_FLAGS+= --size 12  


filesystem/fonts/ru/ru_Montserrat_black.font64: 		MKFONT_FLAGS+= --size 16   --range 20-7F --range 0400-052F
filesystem/fonts/ru/ru_Montserrat_italic.font64: 		MKFONT_FLAGS+= --size 38   --range 20-7F --range 0400-052F
filesystem/fonts/ru/ru_Montserrat_italic_small.font64: 	MKFONT_FLAGS+= --size 16   --range 20-7F --range 0400-052F

filesystem/fonts/jp/NikkyouSans-mLKax_small.font64: 	MKFONT_FLAGS+= --size 18   --range 20-7F --range 30A0-30FF
filesystem/fonts/jp/NikkyouSans-mLKax.font64: 			MKFONT_FLAGS+= --size 38   --range 20-7F --range 30A0-30FF

filesystem/textures/%.sprite: MKSPRITE_FLAGS += --dither ORDERED
filesystem/models/%.sprite:  MKSPRITE_FLAGS += --mipmap BOX

filesystem/music/1_select.wav64:  AUDIOCONV_FLAGS += --wav-compress 3
filesystem/music/7_theme.wav64:  AUDIOCONV_FLAGS += --wav-compress 3
filesystem/music/9_congrats.wav64:  AUDIOCONV_FLAGS += --wav-compress 3