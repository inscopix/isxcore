BUILD_DIR_ROOT=build
BUILD_DIR_MODULES=modules
BUILD_TYPE=Release
BUILD_DIR_CMAKE=cmake
BUILD_PATH=$(BUILD_DIR_ROOT)/$(BUILD_TYPE)/$(BUILD_DIR_CMAKE)

ifeq ($(OS), Windows_NT)
	DETECTED_OS = windows
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		DETECTED_OS = linux
	else ifeq ($(UNAME_S), Darwin)
		DETECTED_OS = mac
	endif
endif

CMAKE_OPTIONS=\
    -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)\
    -DISX_VERSION_MAJOR=${VERSION_MAJOR}\
    -DISX_VERSION_MINOR=${VERSION_MINOR}\
    -DISX_VERSION_PATCH=${VERSION_PATCH}\
    -DISX_VERSION_BUILD=${VERSION_BUILD}\
    -DISX_IS_BETA=${IS_BETA}\
    -DISX_WITH_CUDA=${WITH_CUDA}\
    -DISX_IS_MINIMAL_API=${IS_MINIMAL_API}

ifeq ($(DETECTED_OS), windows)
	CMAKE_GENERATOR = Visual Studio 14 2015 Win64
else ifeq ($(DETECTED_OS), linux)
	CMAKE_GENERATOR = Unix Makefiles
else ifeq ($(DETECTED_OS), mac)
	CMAKE_GENERATOR = Xcode
endif

check_os:
	@echo "Verifying detected OS"
ifndef DETECTED_OS
	@echo "Failed to detect supported OS"; exit 1
else
	@echo "Detected OS: ${DETECTED_OS}"
endif

clean:
	@rm -rf build

build: check_os
	@echo ${CMAKE_GENERATOR} $(BUILD_PATH) $(CMAKE_OPTIONS)
	mkdir -p $(BUILD_PATH) && \
	cd $(BUILD_PATH) && \
	cmake $(CMAKE_OPTIONS) -G $(CMAKE_GENERATOR) ../../../
ifeq ($(DETECTED_OS), mac)
	cd $(BUILD_PATH) && \
	xcodebuild -alltargets -configuration $(BUILD_TYPE) -project Project.xcodeproj CODE_SIGN_IDENTITY=""
endif

rebuild: clean build
