all:
	./install.sh
	WAN_BUILD_FAST=ON ./open_spiel/scripts/build_and_run_tests.sh --build_only=true --wan_build_type=Release
	./build/examples/database_game