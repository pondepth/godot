/**************************************************************************/
/*  os_psp.cpp                                                            */
/**************************************************************************/

#include "os_psp.h"

#include "display_server_psp.h"
#include "ip_psp.h"

#include "drivers/unix/dir_access_unix.h"
#include "drivers/unix/file_access_unix.h"
#include "drivers/unix/thread_posix.h"

#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/os/main_loop.h"
#include "core/profiling/profiling.h"
#include "main/main.h"

#include <pspkernel.h>
#include <psprtc.h>
#include <psputils.h>

#include <cstdio>
#include <cstring>
#include <unistd.h>

void OS_PSP::initialize() {
	init_thread_posix();
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_RESOURCES);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_RESOURCES);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_USERDATA);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_FILESYSTEM);
	IPPSP::make_default();
	ticks_start = sceKernelGetSystemTimeWide();
	DisplayServerPSP::register_psp_driver();
}

void OS_PSP::initialize_joypads() {
}

void OS_PSP::finalize() {
	delete_main_loop();
}

void OS_PSP::finalize_core() {
}

void OS_PSP::set_main_loop(MainLoop *p_main_loop) {
	main_loop = p_main_loop;
}

void OS_PSP::delete_main_loop() {
	if (main_loop) {
		memdelete(main_loop);
		main_loop = nullptr;
	}
}

MainLoop *OS_PSP::get_main_loop() const {
	return main_loop;
}

Vector<String> OS_PSP::get_video_adapter_driver_info() const {
	return Vector<String>();
}

String OS_PSP::get_stdin_string(int64_t) {
	return String();
}

PackedByteArray OS_PSP::get_stdin_buffer(int64_t) {
	return PackedByteArray();
}

Error OS_PSP::get_entropy(uint8_t *r_buffer, int p_bytes) {
	ERR_FAIL_NULL_V(r_buffer, ERR_INVALID_PARAMETER);
	SceKernelUtilsMt19937Context context;
	sceKernelUtilsMt19937Init(&context, static_cast<uint32_t>(sceKernelGetSystemTimeWide()));
	for (int i = 0; i < p_bytes;) {
		const uint32_t value = sceKernelUtilsMt19937UInt(&context);
		const int amount = MIN(4, p_bytes - i);
		memcpy(r_buffer + i, &value, amount);
		i += amount;
	}
	return OK;
}

String OS_PSP::get_name() const {
	return "PSP";
}

String OS_PSP::get_identifier() const {
	return "psp";
}

String OS_PSP::get_distribution_name() const {
	return "PlayStation Portable";
}

String OS_PSP::get_version() const {
	return "homebrew";
}

int OS_PSP::get_processor_count() const {
	return 1;
}

int OS_PSP::get_default_thread_pool_size() const {
	return 1;
}

OS::DateTime OS_PSP::get_datetime(bool p_utc) const {
	pspTime time = {};
	if (p_utc) {
		sceRtcGetCurrentClock(&time, 0);
	} else {
		sceRtcGetCurrentClockLocalTime(&time);
	}
	DateTime result = {};
	result.year = time.year;
	result.month = static_cast<Month>(time.month);
	result.day = time.day;
	result.weekday = WEEKDAY_SUNDAY;
	result.hour = time.hour;
	result.minute = time.minutes;
	result.second = time.seconds;
	result.dst = false;
	return result;
}

OS::TimeZoneInfo OS_PSP::get_time_zone_info() const {
	TimeZoneInfo result;
	result.bias = 0;
	result.name = "UTC";
	return result;
}

void OS_PSP::delay_usec(uint32_t p_usec) const {
	sceKernelDelayThread(p_usec);
}

uint64_t OS_PSP::get_ticks_usec() const {
	return sceKernelGetSystemTimeWide() - ticks_start;
}

String OS_PSP::get_config_path() const {
	return "ms0:/PSP/SAVEDATA";
}

String OS_PSP::get_data_path() const {
	return "ms0:/PSP/SAVEDATA";
}

String OS_PSP::get_cache_path() const {
	return "ms0:/PSP/SAVEDATA";
}

String OS_PSP::get_user_data_dir(const String &p_user_dir) const {
	String directory = p_user_dir.validate_filename();
	if (directory.is_empty()) {
		directory = "GODOT_PSP";
	}
	return "ms0:/PSP/SAVEDATA/" + directory;
}

Error OS_PSP::set_cwd(const String &p_cwd) {
	return chdir(p_cwd.utf8().get_data()) == 0 ? OK : ERR_CANT_OPEN;
}

String OS_PSP::get_cwd() const {
	char buffer[1024];
	return getcwd(buffer, sizeof(buffer)) ? String::utf8(buffer) : String(".");
}

Error OS_PSP::execute(const String &, const List<String> &, String *, int *, bool, Mutex *, bool) {
	return ERR_UNAVAILABLE;
}

Dictionary OS_PSP::execute_with_pipe(const String &, const List<String> &, bool) {
	return Dictionary();
}

Error OS_PSP::create_process(const String &, const List<String> &, ProcessID *, bool) {
	return ERR_UNAVAILABLE;
}

Error OS_PSP::kill(const ProcessID &) {
	return ERR_UNAVAILABLE;
}

bool OS_PSP::is_process_running(const ProcessID &) const {
	return false;
}

int OS_PSP::get_process_id() const {
	return 1;
}

int OS_PSP::get_process_exit_code(const ProcessID &) const {
	return -1;
}

bool OS_PSP::has_environment(const String &) const {
	return false;
}

String OS_PSP::get_environment(const String &) const {
	return String();
}

void OS_PSP::set_environment(const String &, const String &) const {
}

void OS_PSP::unset_environment(const String &) const {
}

void OS_PSP::alert(const String &p_alert, const String &p_title) {
	printf("%s: %s\n", p_title.utf8().get_data(), p_alert.utf8().get_data());
}

bool OS_PSP::_check_internal_feature_support(const String &p_feature) {
	return p_feature == "psp" || p_feature == "mips32";
}

void OS_PSP::run() {
	if (!main_loop) {
		return;
	}

	main_loop->initialize();
	while (true) {
		GodotProfileFrameMark;
		DisplayServer::get_singleton()->process_events();
		if (Main::iteration()) {
			break;
		}
	}
	main_loop->finalize();
}

OS_PSP::OS_PSP() = default;
