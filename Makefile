NAME = drivingstrikers64
 # Internal ROM name
ROM_NAME = "Driving Strikers 64"
BUILD_DIR = build
ASSETS_DIR = assets
FILESYSTEM_DIR = filesystem
T3D_INST=$(shell realpath ../..)

include $(N64_INST)/include/n64.mk
include $(N64_INST)/include/t3d.mk

include assetflags.mk

N64_CFLAGS += -std=gnu2x
N64_C_AND_CXX_FLAGS += -ftrivial-auto-var-init=zero

src = $(shell find ./src/ -type f -name '*.c')

IMAGE_LIST = $(shell find $(ASSETS_DIR)/models/ -type f -name '*.png') \
			 $(shell find $(ASSETS_DIR)/textures/ -type f -name '*.png')
FONT_LIST  = $(shell find $(ASSETS_DIR)/fonts/ -type f -name '*.ttf')
SOUND_LIST  = $(shell find $(ASSETS_DIR)/sfx/ -type f -name '*.wav')
MUSIC_LIST  = $(shell find $(ASSETS_DIR)/music/ -type f -name '*.wav')
MODELS_LIST = $(shell find $(ASSETS_DIR)/models/ -type f -name '*.glb')
TEXT_LIST   = $(shell find $(ASSETS_DIR)/locale/ -type f -name '*.*')
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(IMAGE_LIST:%.png=%.sprite))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(FONT_LIST:%.ttf=%.font64))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(SOUND_LIST:%.wav=%.wav64))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(MUSIC_LIST:%.wav=%.wav64))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(MODELS_LIST:%.glb=%.t3dm))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(TEXT_LIST))

all: $(NAME).z64

$(FILESYSTEM_DIR)/textures/%.sprite: $(ASSETS_DIR)/textures/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) --compress 1 -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/models/%.sprite: $(ASSETS_DIR)/models/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) --compress 1 -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/fonts/%.font64: $(ASSETS_DIR)/fonts/%.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/sfx/%.wav64: $(ASSETS_DIR)/sfx/%.wav
	@mkdir -p $(dir $@)
	@echo "    [SFX] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) --wav-compress 1 -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/music/%.wav64: $(ASSETS_DIR)/music/%.wav
	@mkdir -p $(dir $@)
	@echo "    [MUSIC] $@"
	$(N64_AUDIOCONV) --wav-compress 1,bits=3 --wav-resample 28000 $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/%.t3dm: assets/%.glb
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) "$<" $@
	$(N64_BINDIR)/mkasset -c 2 -o $(FILESYSTEM_DIR) $@

$(FILESYSTEM_DIR)/locale/%: $(ASSETS_DIR)/locale/%
	@mkdir -p $(dir $@)
	@echo "    [TEXT] $@"
	cp "$<" $@

$(BUILD_DIR)/$(NAME).dfs: $(ASSETS_LIST)
$(BUILD_DIR)/$(NAME).elf: $(src:%.c=$(BUILD_DIR)/%.o)

$(NAME).z64: N64_ROM_TITLE=$(ROM_NAME)
$(NAME).z64: $(BUILD_DIR)/$(NAME).dfs
$(NAME).z64: N64_ROM_SAVETYPE = eeprom4k

clean:
	rm -rf $(BUILD_DIR) *.z64
	rm -rf $(FILESYSTEM_DIR)

build_lib:
	rm -rf $(BUILD_DIR) *.z64
	make -C $(T3D_INST)
	make all

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
