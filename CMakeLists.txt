include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

target_sources(app PRIVATE
	       src/main.c
	       src/on_chip_temp.c
	       src/bt_gatt_read.c)
