fn OptimizeMesh maxFace = 
(
	for OBJ in Geometry do(
		InstanceMgr.MakeObjectsUnique OBJ #prompt
		local numfaces = OBJ.mesh.numfaces

		if numfaces > 256 then
		(	
			if numfaces < maxFace then
			(
				local vp = 50
			)
			else
			(
				local vp = maxFace * 100 / numfaces
			)			
			
			proOpt = ProOptimizer()
			addmodifier OBJ proOpt
			proOpt.Calculate = on
			proOpt.OptimizationMode = 1
			proOpt.KeepUV = on
			proOpt.VertexPercent = vp
			mSmooth = smooth ()
			addmodifier OBJ mSmooth
			mSmooth.autosmooth  = on
			proOpt.Calculate = on
			mSmooth.enabled = false
		)
	)
)

fn AutoUnwrapUV =
(
	max modify mode
	local unwrapUVW_normalList = #([1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]) -- arg for Unwrap_UVW.flattenMap
	for OBJ in Geometry do(		
		unwrapMod = unwrap_UVW()
		
		unwrapMod.setAlwaysEdit ( false )
		unwrapMod.setMapChannel ( 2 )
		unwrapMod.setFlattenAngle ( 60 )
		unwrapMod.setFlattenSpacing( 0.02 )
		--unwrapMod.setFlattenNormalize( commonBakeProps.cNormalize.checked )
		unwrapMod.setFlattenNormalize( true )
		unwrapMod.setFlattenRotate( true )
		unwrapMod.setFlattenFillHoles( true )
		unwrapMod.setApplyToWholeObject( true )
		unwrapMod.name = "Automatic Flatten UVs"
		select OBJ
		addModifier OBJ unwrapMod
		
		unwrapMod.flattenMap 60	unwrapUVW_normalList 0.02 true 0 true true 
	)
)

fn BakeDiffuseAndAlpha obj size pName=
(
	--Clear all render elements 
	obj.iNodeBakeProperties.removeAllBakeElements() 
	--Preparing the Bake Elements:
	be1 = VRayDiffuseFilterMap() --instance of the bake element class
	be1.outputSzX = be1.outputSzY = size --set the size of the baked map --specifythe full file path, name and type:
	be1.fileType = (pName + obj.name +"_diffuse.png")
	--be1.fileName = filenameFromPath be1.fileType
	be1.filterOn = true --enable filtering
	--be1.shadowsOn = false --disable shadows
	--be1.lightingOn = false --disable lighting
	be1.enabled = true --enable baking
	be2 = AlphaMap() -- instance of the bake element class
	be2.outputSzX =be2.outputSzY = size --set the size of the baked map --specifythe full file path, name and type:
	be2.fileType = ("E:/_alpha.png")
	be2.fileName = filenameFromPath be2.fileType
	be2.filterOn = true --enable filtering
	--be2.shadowsOn =true --enable shadows
	be2.enabled = true --enable baking --Preparing theobjectfor baking:
	obj.INodeBakeProperties.addBakeElement be1 --add first element
	--obj.INodeBakeProperties.addBakeElement be2 --add second element
	obj.INodeBakeProperties.bakeEnabled = true --enabling baking
	obj.INodeBakeProperties.bakeChannel = 2 --channel to bake
	obj.INodeBakeProperties.nDilations = 1 --expand the texturea bit
	select obj --we are baking the selection, so we select the object --Call the renderer to bake both elements:
	render rendertype:#bakeSelected outputSize:[size,size] vfb:off
	obj.INodeBakeProperties.RemoveBakeElement be1
	
	obj.material = Standardmaterial()
	--obj.material.diffuseMap = bitmaptexture filename:(filenameFromPath be1.fileType)
	obj.material.diffuseMap = bitmaptexture filename:be1.fileType
	obj.material.diffuseMap.coords.mapChannel = 1
	obj.material.name = obj.name +"_mat"
	obj.modifiers[#Automatic_Flatten_UVs].unwrap.setMapChannel 1
	try(convertToMesh obj) catch()
	
)--end fn

fn BackTexture pName=
(
	renderers.current.output_on = false
	renderers.current.imageSampler_type = 3
	renderers.current.progressive_maxSamples = 4
	renderers.current.gi_on = false
	for OBJ in Geometry do(
		
		BakeDiffuseAndAlpha OBJ 2048 pName
	)
)

fn ExportMeshs pName=
(
	local theClasses = exporterPlugin.classes
	local _objIdx = findItem theClasses ObjExp
	local objPath = pName + "\\OBJ\\export.obj"
	local packagePath = pName + "\\OBJ\\export_obj.zip"
	-- with *.OBJ export dialog
	--exportFile (pName) using:theClasses[_objIdx]
	-- without *.OBJ export dialog
	local z7Path = pName + "\\7z.exe"
	local filesPath = pName + "\\OBJ\\*.*"
	exportFile objPath #noprompt selectedOnly:off using:theClasses[_objIdx]
	HiddenDOSCommand (z7Path + " a -tzip " + packagePath + " " + filesPath)
	return packagePath
)

fn AttachMeshs =
(
	local meshNames = #()
	for OBJ in Geometry do(
		try(
			--resetxform OBJ
			convertToPoly OBJ
			if isKindOf OBJ Editable_Poly then
			(
				append meshNames OBJ
			)
		) catch()
	)
	
	for i = 2 to meshNames.count do
	(
		 polyop.attach meshNames[1] meshNames[i]
	)
)

fn ExportESS pName=
(
	max select all
	local essPath = pName + "\\ESS\\export.ess"
	renderers.current = Elara_Renderer()
	renderers.current.UseRelativePath = false
	renderers.current.ExportSelection = true
	if renderers.current.export_ess(essPath) then print "Exported ESS."
	renderers.current = V_Ray_Adv_3_60_03()
	
	local erPath = pName + "\\ElaraSDK\\bin\\er.exe"
	local packagePath = pName + "\\ESS\\export_ess.zip"
	HiddenDOSCommand (erPath + " -make_package " + essPath + " " + packagePath)
	return packagePath
)

fn ReadMaxScene pName = 
(
	resetMaxFile #noprompt
	units.displaytype = #metric
	units.MetricType = #Millimeters
	units.SystemType = #Millimeters
	
	loadMaxFile pName useFileUnits:false quiet:true
)

fn UploadESSAndObj essPack objPack =
(
	local uplocalCommand = ("UploadESSAndObj " + essPack + " " + objPack)
	print uplocalCommand
	HiddenDOSCommand uplocalCommand
)

fn CheckScene =
(
	max select all
	ungroup selection
	local helperObjs = #()
	for o in objects where superclassof o != GeometryClass do
	(
		append helperObjs o
	)
	delete helperObjs
)

fn AlignScene =
(
	max select all
	bigGroup = group selection
	bigGroup.pos = [0, 0, 0]
)

fn CallMeshTool maxScenename meshToolPath=
(
	ReadMaxScene maxScenename
	CheckScene()
	AlignScene()
	local essPack = ExportESS meshToolPath
	OptimizeMesh 5000
	AttachMeshs()
	AutoUnwrapUV()
	BackTexture (meshToolPath + "/OBJ/")
	local objPack = ExportMeshs meshToolPath
	UploadESSAndObj essPack objPack
)