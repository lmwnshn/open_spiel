all:
	./install.sh
	./open_spiel/scripts/build_and_run_tests.sh --build_only=true
	./build/examples/cfr_db