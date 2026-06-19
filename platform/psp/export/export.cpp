/**************************************************************************/
/*  export.cpp                                                            */
/**************************************************************************/

#include "export.h"

#include "export_plugin.h"

#include "core/object/class_db.h"
#include "editor/export/editor_export.h"

void register_psp_exporter_types() {
	GDREGISTER_VIRTUAL_CLASS(EditorExportPlatformPSP);
}

void register_psp_exporter() {
	Ref<EditorExportPlatformPSP> platform;
	platform.instantiate();
	EditorExport::get_singleton()->add_export_platform(platform);
}

