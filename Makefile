BUILD_DIR=build
include $(N64_INST)/include/n64.mk

src = brew-reality.c
assets_xm = $(wildcard assets/*.xm)
assets_wav = $(wildcard assets/*.wav)
assets_png = $(wildcard assets/*.png)
assets_ttf = $(wildcard assets/*.ttf)
assets_model64 = $(wildcard assets/*.gltf)

assets_conv = $(addprefix filesystem/,$(notdir $(assets_xm:%.xm=%.xm64))) \
              $(addprefix filesystem/,$(notdir $(assets_wav:%.wav=%.wav64))) \
			  $(addprefix filesystem/,$(notdir $(assets_ttf:%.ttf=%.font64))) \
              $(addprefix filesystem/,$(notdir $(assets_png:%.png=%.sprite))) \
			  $(addprefix filesystem/,$(notdir $(assets_model64:%.gltf=%.model64)))

#N64_CFLAGS = -Wno-error
AUDIOCONV_FLAGS ?=
MKSPRITE_FLAGS ?=

all: brew-reality.z64

filesystem/%.xm64: assets/%.xm
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o filesystem $<

filesystem/%.wav64: assets/%.wav
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) --wav-compress 0 -o filesystem $<

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	@$(N64_MKSPRITE) --compress 0 --texparms 0,0,0,0,inf,inf,0,0 $(MKSPRITE_FLAGS) -o "$(dir $@)" "$<"

filesystem/beach01.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-0,-1,inf,inf,1,1
filesystem/grass01.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-0,-1,inf,inf,1,1
filesystem/rocks01.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-0,-1,inf,inf,1,1
filesystem/trees01.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-0,-1,inf,inf,1,1

filesystem/beach04.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,16,-1,-0,inf,inf,1,1
filesystem/beach02.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-1,-0,inf,inf,1,1
filesystem/trees01.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-1,-0,inf,inf,1,1
filesystem/beach03.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-1,-0,inf,inf,1,1
filesystem/water01.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-1,-0,inf,inf,1,1
filesystem/tree.rgba32.sprite: MKSPRITE_FLAGS= --format RGBA32 --mipmap BOX --texparms 0,0,1,1,inf,inf,1,1
filesystem/jetnose01.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-2,-1,inf,inf,1,1
filesystem/jettop01.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-2,-1,inf,inf,1,1
filesystem/jetwings01.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-2,-1,inf,inf,1,1
filesystem/jetbottom01.ihq.sprite: MKSPRITE_FLAGS= --texparms 0,0,-1,-2,inf,inf,1,1

filesystem/stars.i4.sprite: MKSPRITE_FLAGS= --mipmap NONE
filesystem/clouds.i4.sprite: MKSPRITE_FLAGS= --mipmap NONE

filesystem/%.font64: assets/%.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	@$(N64_MKFONT) $(MKFONT_FLAGS) --compress 0 -o filesystem "$<"

filesystem/%.model64: assets/%.gltf
	@mkdir -p $(dir $@)
	@echo "    [MODEL64] $@"
	@$(N64_MKMODEL) -o filesystem "$<"

$(BUILD_DIR)/brew-reality.dfs: $(assets_conv)
$(BUILD_DIR)/brew-reality.elf: $(src:%.c=$(BUILD_DIR)/%.o)

brew-reality.z64: N64_ROM_TITLE="Homebrew Reality"
brew-reality.z64: $(BUILD_DIR)/brew-reality.dfs

clean:
	rm -rf $(BUILD_DIR) filesystem/ brew-reality.z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
