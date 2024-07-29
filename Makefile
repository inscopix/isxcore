.PHONY: build test

# Variables defining build paths
BUILD_DIR_ROOT=build
BUILD_DIR_MODULES=modules
BUILD_TYPE=Release
BUILD_DIR_CMAKE=cmake
BUILD_PATH=$(BUILD_DIR_ROOT)/$(BUILD_TYPE)/$(BUILD_DIR_CMAKE)

# Users can optionally define paths to third party and test data dirs
# if it's not placed within this repo
ifndef THIRD_PARTY_DIR
	THIRD_PARTY_DIR=third_party
endif

ifndef TEST_DATA_DIR
	TEST_DATA_DIR=test_data
endif

# Variables for unit tests
BIN_DIR=bin
MOSTEST_EXE=mostest
MOSTEST_BIN_DIR=$(BUILD_DIR_ROOT)/$(BUILD_TYPE)/$(BIN_DIR)
MOSTEST_COMMAND=$(MOSTEST_BIN_DIR)/$(MOSTEST_EXE) -p $(TEST_DATA_DIR) $*

# Detect OS from env variables or uname
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

# Environment variables to pass to cmake for compilation
# Configures the lib version
ISX_VERSION_MAJOR=1
ISX_VERSION_MINOR=9
ISX_VERSION_PATCH=5
ISX_VERSION_BUILD=0
ISX_IS_BETA=1

# Configures building with algos module from IDPS
ISX_WITH_ALGOS=0

CMAKE_OPTIONS=\
    -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)\
    -DISX_VERSION_MAJOR=${ISX_VERSION_MAJOR}\
    -DISX_VERSION_MINOR=${ISX_VERSION_MINOR}\
    -DISX_VERSION_PATCH=${ISX_VERSION_PATCH}\
    -DISX_VERSION_BUILD=${ISX_VERSION_BUILD}\
    -DISX_IS_BETA=${ISX_IS_BETA}\
    -DISX_WITH_ALGOS=${ISX_WITH_ALGOS}

ifeq ($(DETECTED_OS), windows)
	CMAKE_GENERATOR = Visual Studio 14 2015 Win64
else ifeq ($(DETECTED_OS), linux)
	CMAKE_GENERATOR = Unix Makefiles
	CMAKE_OPTIONS += -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
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

setup:
	./scripts/setup -v --src ${REMOTE_DIR} --dst ${REMOTE_LOCAL_DIR} --remote-copy
	./scripts/setup --src ${IDPS_REMOTE_EXT_COPY_DIR}

build: check_os
	@echo ${CMAKE_GENERATOR} $(BUILD_PATH) $(CMAKE_OPTIONS) $(THIRD_PARTY_DIR)
	@mkdir -p $(BUILD_PATH) && \
	cd $(BUILD_PATH) && \
	THIRD_PARTY_DIR=$(THIRD_PARTY_DIR) cmake $(CMAKE_OPTIONS) -G "$(CMAKE_GENERATOR)" ../../../
ifeq ($(DETECTED_OS), windows)
	@cd $(BUILD_PATH) && \
	"/c/Program Files (x86)/MSBuild/14.0/Bin/MSBuild.exe" isxcore.sln //p:Configuration=$(BUILD_TYPE) //maxcpucount:8
else ifeq ($(DETECTED_OS), linux)
	@cd $(BUILD_PATH) && \
	make -j2
else ifeq ($(DETECTED_OS), mac)
	@cd $(BUILD_PATH) && \
	xcodebuild -alltargets -configuration $(BUILD_TYPE) -project isxcore.xcodeproj CODE_SIGN_IDENTITY=""
endif

rebuild: clean build
 
test: build
ifeq ($(DETECTED_OS), windows)
	@$(MOSTEST_COMMAND)
else ifeq ($(DETECTED_OS), mac)
	@DYLD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(MOSTEST_BIN_DIR) $(MOSTEST_COMMAND)
else ifeq ($(DETECTED_OS), linux)
	@LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(MOSTEST_BIN_DIR) ${MOSTEST_COMMAND}
endif
