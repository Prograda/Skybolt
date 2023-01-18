bl_info = {
	"name": "Skybolt Tools",
	"author": "Matthew Reid",
	"version": (1, 0, 0),
	"blender": (2, 81, 6),
	"location": "3D Viewport Tools Panel",
	"description": "Tools for Skybolt",
	"category": "Scene",
}

if "bpy" in locals():
	import importlib
	importlib.reload(export_skybolt)
	importlib.reload(skybolt_tools_panel)
else:
	from . import export_skybolt, skybolt_tools_panel

classes = [
	export_skybolt.ExportSceneToSkybolt,
	export_skybolt.ExportSceneToSkyboltProperties,
	skybolt_tools_panel.CreateTransformFromSelected,
	skybolt_tools_panel.PrintSelectedTransformRelSecond,
	skybolt_tools_panel.SkyboltToolPanel
]

import bpy

def register():
	for cls in classes:
		bpy.utils.register_class(cls)

	bpy.types.Scene.exportSceneToSkyboltProperties = bpy.props.PointerProperty(type=export_skybolt.ExportSceneToSkyboltProperties)

def unregister():
	for cls in classes:
		bpy.utils.unregister_class(cls)

	del bpy.types.Scene.exportSceneToSkyboltProperties

if __name__ == "__main__":
	register()
