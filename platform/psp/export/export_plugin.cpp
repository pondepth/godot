/**************************************************************************/
/*  export_plugin.cpp                                                     */
/**************************************************************************/

#include "export_plugin.h"

#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"

String EditorExportPlatformPSP::_template_name(bool p_debug) const {
	return p_debug ? "psp_debug.pbp" : "psp_release.pbp";
}

String EditorExportPlatformPSP::_get_template_path(const Ref<EditorExportPreset> &p_preset, bool p_debug, String *r_error) const {
	const String option = p_debug ? "custom_template/debug" : "custom_template/release";
	const String custom_template = p_preset->get(option);
	if (!custom_template.is_empty()) {
		if (!FileAccess::exists(custom_template)) {
			if (r_error) {
				*r_error = vformat(TTR("Custom PSP template not found: %s"), custom_template);
			}
			return String();
		}
		return custom_template;
	}
	return find_export_template(_template_name(p_debug), r_error);
}

void EditorExportPlatformPSP::get_preset_features(const Ref<EditorExportPreset> &, List<String> *r_features) const {
	r_features->push_back("psp");
	r_features->push_back("mips32");
	r_features->push_back("32");
}

void EditorExportPlatformPSP::get_export_options(List<ExportOption> *r_options) const {
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "custom_template/debug", PROPERTY_HINT_GLOBAL_FILE, "*.pbp"), ""));
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "custom_template/release", PROPERTY_HINT_GLOBAL_FILE, "*.pbp"), ""));
}

bool EditorExportPlatformPSP::has_valid_export_configuration(const Ref<EditorExportPreset> &p_preset, String &r_error, bool &r_missing_templates, bool p_debug) const {
	r_missing_templates = false;
	const String option = p_debug ? "custom_template/debug" : "custom_template/release";
	const String custom_template = p_preset->get(option);
	const String path = _get_template_path(p_preset, p_debug, &r_error);
	if (path.is_empty()) {
		r_missing_templates = custom_template.is_empty();
		return false;
	}
	return true;
}

bool EditorExportPlatformPSP::has_valid_project_configuration(const Ref<EditorExportPreset> &, String &r_error) const {
	const int width = GLOBAL_GET("display/window/size/viewport_width");
	const int height = GLOBAL_GET("display/window/size/viewport_height");
	if (width != 480 || height != 272) {
		r_error = TTR("PSP projects should use a 480 x 272 viewport. The exporter will continue, but scaling is not implemented by the experimental renderer.");
	}
	return true;
}

List<String> EditorExportPlatformPSP::get_binary_extensions(const Ref<EditorExportPreset> &) const {
	List<String> extensions;
	extensions.push_back("PBP");
	return extensions;
}

Error EditorExportPlatformPSP::export_project(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, BitField<EditorExportPlatform::DebugFlags> p_flags, bool p_notify) {
	ExportNotifier notifier(*this, p_preset, p_debug, p_path, p_flags, p_notify);

	String template_error;
	const String template_path = _get_template_path(p_preset, p_debug, &template_error);
	if (template_path.is_empty()) {
		add_message(EXPORT_MESSAGE_ERROR, TTR("Prepare Templates"), template_error);
		return ERR_FILE_NOT_FOUND;
	}

	const String output_dir = p_path.get_base_dir();
	Error err = DirAccess::make_dir_recursive_absolute(output_dir);
	if (err != OK) {
		add_message(EXPORT_MESSAGE_ERROR, TTR("Export"), vformat(TTR("Could not create output directory: %s"), output_dir));
		return err;
	}

	err = DirAccess::copy_absolute(template_path, p_path);
	if (err != OK) {
		add_message(EXPORT_MESSAGE_ERROR, TTR("Export"), TTR("Could not copy the PSP EBOOT template."));
		return err;
	}

	const String pack_path = output_dir.path_join("data.pck");
	err = save_pack(p_preset, p_debug, pack_path);
	if (err != OK) {
		add_message(EXPORT_MESSAGE_ERROR, TTR("Export"), TTR("Could not create data.pck."));
		return err;
	}

	add_message(EXPORT_MESSAGE_INFO, TTR("Export"), vformat(TTR("Created %s and %s. Copy both files to /PSP/GAME/<game>/ on the Memory Stick."), p_path.get_file(), pack_path.get_file()));
	return OK;
}

void EditorExportPlatformPSP::get_platform_features(List<String> *r_features) const {
	r_features->push_back("psp");
	r_features->push_back("mips32");
	r_features->push_back("32");
}

HashMap<String, Variant> EditorExportPlatformPSP::get_custom_project_settings(const Ref<EditorExportPreset> &) const {
	HashMap<String, Variant> settings;
	settings["display/window/size/viewport_width"] = 480;
	settings["display/window/size/viewport_height"] = 272;
	settings["display/window/size/window_width_override"] = 480;
	settings["display/window/size/window_height_override"] = 272;
	settings["rendering/renderer/rendering_method"] = "dummy";
	settings["rendering/renderer/rendering_method.mobile"] = "dummy";
	return settings;
}

void EditorExportPlatformPSP::initialize() {
	if (!EditorNode::get_singleton()) {
		return;
	}
	Ref<Theme> theme = EditorNode::get_singleton()->get_editor_theme();
	if (theme.is_valid()) {
		logo = theme->get_icon(SNAME("JoyButton"), EditorStringName(EditorIcons));
		if (logo.is_null()) {
			logo = theme->get_icon(SNAME("DefaultProjectIcon"), EditorStringName(EditorIcons));
		}
	}
}
