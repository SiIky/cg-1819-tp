GENERATE := generator/generate
ENGINE := engine/engine

ASSETS := \
    assets/box.3d      \
    assets/cone.3d     \
    assets/cylinder.3d \
    assets/sphere.3d   \
    assets/teapot.3d   \
    assets/terra.jpg   \

run: $(ENGINE) engine/scene_solar_system.xml $(ASSETS)
	cd engine/ && ./engine scene_solar_system.xml

$(ENGINE): engine/Makefile
	make -C engine/ -j

engine/Makefile:
	cd engine/ && cmake CMakeLists.txt

engine/scene.xml: $(ASSETS)

assets/:
	mkdir -p assets/

$(GENERATE): generator/Makefile
	make -C generator/ -j

generator/Makefile:
	cd generator/ && cmake CMakeLists.txt

engine/scene_solar_system.xml: assets/sphere.3d assets/teapot.3d assets/terra.jpg

assets/box.3d: assets/ $(GENERATE)
	$(GENERATE) box $@ 4 4 4 2

assets/cone.3d: assets/ $(GENERATE)
	$(GENERATE) cone $@ 4 6 20 10

assets/cylinder.3d: assets/ $(GENERATE)
	$(GENERATE) cylinder $@ 40 6 20 10

assets/sphere.3d: assets/ $(GENERATE)
	$(GENERATE) sphere $@ 4 20 20

assets/teapot.3d: assets/ $(GENERATE) teapot.patch
	$(GENERATE) bezier $@ teapot.patch 2 2

clean:
	$(RM) $(ASSETS)
	cd engine/ && make clean
	cd generator/ && make clean

tokei:
	tokei engine/*.cpp engine/*.h generator/*.cpp generator/*.h

.PHONY: run clean tokei $(ENGINE) $(GENERATE)
