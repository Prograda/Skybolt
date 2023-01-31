import bpy
from io_scene_obj import export_obj
import mathutils
import math
import os
import shutil
from subprocess import Popen, PIPE, STDOUT
import re

def hasExportableMeshesInHierarchy(object):
	if object.type == 'MESH' and not object.name.lower().endswith('no_export'):
		return True
	    
	for c in object.children:
		if hasExportableMeshesInHierarchy(c):
			return True
												
	return False

class ExportSceneToSkyboltProperties(bpy.types.PropertyGroup):
	assetName : bpy.props.StringProperty(name="Asset Name")
	outputDir : bpy.props.StringProperty(name="Output Dir", default="//export", subtype = 'DIR_PATH')

class ExportSceneToSkybolt(bpy.types.Operator):
	"""Exports scene to IVE files and textures which can be read by Skybolt.
Generates OBJ and MTL files as an intermediate step. Only exports visible
objects parented under transforms with recognized names."""
	bl_idname = "wm.export_scene_to_skybolt"
	bl_label = "Export Scene to Skybolt"
	
	@classmethod
	def poll(self, context):
		properties = context.scene.exportSceneToSkyboltProperties
		return properties != None and properties.assetName != ""
	
	def execute(self, context):

		self.osgConvPath = self.findOsgConvExecutable()
		if self.osgConvPath == None:
			self.report({'ERROR'}, "Could not find osgconv application in path")
			return {'CANCELLED'}

		props = context.scene.exportSceneToSkyboltProperties
		assetName = props.assetName

		directory = bpy.path.abspath(props.outputDir).replace('\\', '/')

		os.makedirs(directory, exist_ok=True)
		
		for object in bpy.data.objects:
			if object.parent == None:
				if hasExportableMeshesInHierarchy(object):
					partName = assetName + "_" + object.name

					filenameWithoutExtension = directory + "/" + partName

					self.exportHierarchyToObj(context, object, filenameWithoutExtension + ".obj")
					self.exportTexturesInMtlFile(filenameWithoutExtension + ".mtl")
					self.convertObjToIve(directory, partName)

		return {'FINISHED'}
	

	def findOsgConvExecutable(self):
		names = ["osgconv.exe", "osgconvrd.exe", "osgconv"]
		for name in names:
			if shutil.which(name):
				return name
		return None
	
	def convertObjToIve(self, directory, name):
		
		# Merge Geodes and Geometry for better render performance.
		# MERGE_GEOMETRY only merges wtihin a geode, and becase OSG imports Obj objects
		# as different geodes, MERGE_GEODES is required as well.
		os.environ["OSG_OPTIMIZER"] = "DEFAULT_OPTIMIZATIONS|MERGE_GEODES|MERGE_GEOMETRY"
		
		outputFilename = directory + "/" + name + ".osgb"
		args = [
			self.osgConvPath,
			'-O',
			'WriteImageHint=UseExternal',
			directory + "/" + name + ".obj",
			outputFilename]
		
		print("Converting OBJ to OSGB with args: " + str(args))
		
		p = Popen(args, cwd=directory, shell=True, stdin=PIPE, stdout=PIPE, stderr=STDOUT, close_fds=True)
		result = str(p.stdout.read())
		result = result.replace("\\r\\n", "\n")
		result = result.replace('\\n', '\n')
		print(result)
	
	def selectVisibleChildrenRecursive(self, object):
		for obj in object.children:
			self.selectVisibleChildrenRecursive(obj)
			if obj.visible_get():
				obj.select_set(True)

	def exportHierarchyToObj(self, context, object, outFilename):
		bpy.ops.object.select_all(action='DESELECT')
		self.selectVisibleChildrenRecursive(object)
		
		originMatrix =  object.matrix_world

		self.exportSelectedToObj(context, originMatrix, outFilename)
		
	def exportSelectedToObj(self, context, originMatrix, filename):

		invOriginMatrix = originMatrix.copy()

		# Convert to North-East-Down coordinates
		invOriginMatrix = invOriginMatrix @ mathutils.Matrix.Rotation(math.radians(-90), 4, 'X')
		invOriginMatrix = invOriginMatrix @ mathutils.Matrix.Rotation(math.radians(-90), 4, 'Y')
		
		# Invert matrix to export mesh in the space of the origin
		invOriginMatrix.invert()

		keywords = {
			'filepath':filename,
			'use_selection':True,
			'use_animation':False,
			'use_mesh_modifiers':True,
			'use_edges':True,
			'use_smooth_groups':True,
			'use_smooth_groups_bitflags':False,
			'use_normals':True,
			'use_uvs':True,
			'use_materials':True,
			'use_triangles':False,
			'use_nurbs':False,
			'use_vertex_groups':False,
			'use_blen_objects':False,
			'group_by_object':False,
			'group_by_material':True,
			'keep_vertex_order':False,
			'global_matrix':invOriginMatrix,
			'path_mode':'AUTO'}

		export_obj.save(context, **keywords)
		
		self.makeTexturePathsRelative(filename.replace(".obj", ".mtl"))
		
	def exportTexturesInMtlFile(self, mtlFilename):
		with open(mtlFilename, 'r') as file :
			fileData = file.read()
			p = re.compile('map_.* (.*)')
			textures = p.findall(fileData)
			if textures:
				sourceDir = os.path.dirname(bpy.data.filepath);
				destinationtDir = os.path.dirname(mtlFilename);
				
				for texture in textures:
					sourcePath = sourceDir + "/" + texture
					destinationPath = destinationtDir + "/" + texture
					
					print("Copying file from '" + sourcePath + "' to '" + destinationPath + "'")
					
					os.makedirs(os.path.dirname(destinationPath), exist_ok=True)
					shutil.copyfile(sourcePath, destinationPath)
		
	def makeTexturePathsRelative(self, mtlFilename):
		dir = os.path.dirname(bpy.data.filepath).replace('\\', '/')

		with open(mtlFilename, 'r') as file :
		  fileData = file.read()

		fileData = fileData.replace('\\\\', '/')
		fileData = fileData.replace('\\', '/')
		fileData = fileData.replace(dir + '/', '')
		fileData = fileData.replace(dir, '')

		with open(mtlFilename, 'w') as file:
		  file.write(fileData)
