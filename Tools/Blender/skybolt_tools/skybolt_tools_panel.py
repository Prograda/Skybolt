import bpy
import math

class CreateTransformFromSelected(bpy.types.Operator):
	"""Create an Empty with transform equal to the selected object"""
	bl_idname = "wm.skybolt_create_transform_from_selected"
	bl_label = "Create Transform"

	@classmethod
	def poll(cls, context):
		return bpy.context.active_object != None
	
	def execute(self, context):
		matrix = bpy.context.active_object.matrix_world
		bpy.ops.object.empty_add(type='PLAIN_AXES', align='WORLD', location=(0, 0, 0))
		bpy.context.selected_objects[0].matrix_world = matrix

		return {'FINISHED'}

class PrintSelectedTransformRelSecond(bpy.types.Operator):
	"""Print the transform of active selected object
in the space of the second object in the selection"""
	bl_idname = "wm.skybolt_print_selected_transform_rel_second"
	bl_label = "Print Transform Rel Second"

	@classmethod
	def poll(cls, context):
		return len(bpy.context.selected_objects) >= 2
	
	def execute(self, context):
		object = bpy.context.active_object
		parent = bpy.context.selected_objects[0]
		if (parent == object):
			parent = bpy.context.selected_objects[1]
		
		invParent = parent.matrix_world.copy()
		invParent.invert()
		
		result = invParent @ object.matrix_world
			
		print("Transform of " + object.name + " relative to " + parent.name + ":")
		print("Translation: " + str(result.to_translation()))
		
		rotation = result.to_quaternion().to_axis_angle()
		print("Rotation Axis: " + str(rotation[0]))
		print("Rotation Angle: " + str(math.degrees(rotation[1])))

		return {'FINISHED'}

class SkyboltToolPanel(bpy.types.Panel):
	"""Creates a Panel in the Object properties window"""
	bl_label = "Skybolt Tools Panel"
	bl_idname = "OBJECT_PT_skybolt_tools"
	bl_space_type = 'VIEW_3D'
	bl_region_type = 'UI'
	bl_category = "Skybolt"

	def draw(self, context):
		layout = self.layout

		row = layout.row()
		row.operator("wm.skybolt_create_transform_from_selected")
		
		row = layout.row()
		row.operator("wm.skybolt_print_selected_transform_rel_second")
		
		# Export Scene
		box = layout.box()
		
		props = context.scene.exportSceneToSkyboltProperties
		row = box.row()
		row.prop(props, "assetName")
		row = box.row()
		row.prop(props, "outputDir")
		
		row = box.row()
		row.operator("wm.export_scene_to_skybolt")
