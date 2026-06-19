/**************************************************************************/
/*  os_psp.h                                                              */
/**************************************************************************/

#pragma once

#include "core/os/os.h"

class OS_PSP : public OS {
	MainLoop *main_loop = nullptr;
	uint64_t ticks_start = 0;

protected:
	void initialize() override;
	void initialize_joypads() override;
	void finalize() override;
	void finalize_core() override;
	void set_main_loop(MainLoop *p_main_loop) override;
	void delete_main_loop() override;
	bool _check_internal_feature_support(const String &p_feature) override;

public:
	Vector<String> get_video_adapter_driver_info() const override;
	String get_stdin_string(int64_t p_buffer_size = 1024) override;
	PackedByteArray get_stdin_buffer(int64_t p_buffer_size = 1024) override;
	Error get_entropy(uint8_t *r_buffer, int p_bytes) override;

	String get_name() const override;
	String get_identifier() const override;
	String get_distribution_name() const override;
	String get_version() const override;

	MainLoop *get_main_loop() const override;
	int get_processor_count() const override;
	int get_default_thread_pool_size() const override;
	DateTime get_datetime(bool p_utc = false) const override;
	TimeZoneInfo get_time_zone_info() const override;
	void delay_usec(uint32_t p_usec) const override;
	uint64_t get_ticks_usec() const override;

	String get_config_path() const override;
	String get_data_path() const override;
	String get_cache_path() const override;
	String get_user_data_dir(const String &p_user_dir) const override;
	Error set_cwd(const String &p_cwd) override;
	String get_cwd() const override;

	Error execute(const String &p_path, const List<String> &p_arguments, String *r_pipe = nullptr, int *r_exitcode = nullptr, bool p_read_stderr = false, Mutex *p_pipe_mutex = nullptr, bool p_open_console = false) override;
	Dictionary execute_with_pipe(const String &p_path, const List<String> &p_arguments, bool p_blocking = true) override;
	Error create_process(const String &p_path, const List<String> &p_arguments, ProcessID *r_child_id = nullptr, bool p_open_console = false) override;
	Error kill(const ProcessID &p_pid) override;
	bool is_process_running(const ProcessID &p_pid) const override;
	int get_process_id() const override;
	int get_process_exit_code(const ProcessID &p_pid) const override;
	bool has_environment(const String &p_var) const override;
	String get_environment(const String &p_var) const override;
	void set_environment(const String &p_var, const String &p_value) const override;
	void unset_environment(const String &p_var) const override;

	void alert(const String &p_alert, const String &p_title = "ALERT!") override;
	void run();

	OS_PSP();
};
