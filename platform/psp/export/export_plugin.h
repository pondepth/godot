/**************************************************************************/
/*  export_plugin.h                                                       */
/**************************************************************************/

#pragma once

#include "editor/export/editor_export_platform.h"
#include "scene/resources/texture.h"

class EditorExportPlatformPSP : public EditorExportPlatform {
	GDCLASS(EditorExportPlatformPSP, EditorExportPlatform);

	Ref<Texture2D> logo;

	String _template_name(bool p_debug) const;
	String _get_template_path(const Ref<EditorExportPreset> &p_preset, bool p_debug, String *r_error = nullptr) const;

public:
	void get_preset_features(const Ref<EditorExportPreset> &p_preset, List<String> *r_features) const override;
	void get_export_options(List<ExportOption> *r_options) const override;

	String get_name() const override { return "PSP"; }
	String get_os_name() const override { return "PSP"; }
	Ref<Texture2D> get_logo() const override { return logo; }

	bool has_valid_export_configuration(const Ref<EditorExportPreset> &p_preset, String &r_error, bool &r_missing_templates, bool p_debug = false) const override;
	bool has_valid_project_configuration(const Ref<EditorExportPreset> &p_preset, String &r_error) const override;
	List<String> get_binary_extensions(const Ref<EditorExportPreset> &p_preset) const override;
	Error export_project(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, BitField<EditorExportPlatform::DebugFlags> p_flags = 0, bool p_notify = true) override;

	void get_platform_features(List<String> *r_features) const override;
	HashMap<String, Variant> get_custom_project_settings(const Ref<EditorExportPreset> &p_preset) const override;
	void initialize() override;
};
