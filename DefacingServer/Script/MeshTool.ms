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
	print ("Finished " + "OptimizeMesh")
)

fn AutoUnwrapUV =
(
	max modify mode
	local unwrapUVW_normalList = #([1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]) -- arg for Unwrap_UVW.flattenMap
	for OBJ in Geometry do(		
		unwrapMod = unwrap_UVW()
		
		unwrapMod.setAlwaysEdit ( false )
		unwrapMod.setMapChannel ( 2 )
		unwrapMod.setFlattenAngle ( 20 )
		unwrapMod.setFlattenSpacing( 0.001 )
		--unwrapMod.setFlattenNormalize( commonBakeProps.cNormalize.checked )
		unwrapMod.setFlattenNormalize( true )
		unwrapMod.setFlattenRotate( true )
		unwrapMod.setFlattenFillHoles( true )
		unwrapMod.setApplyToWholeObject( true )
		unwrapMod.name = "Automatic Flatten UVs"
		select OBJ
		addModifier OBJ unwrapMod
		
		unwrapMod.flattenMap 20	unwrapUVW_normalList 0.001 true 0 true true 
	)
	print ("Finished " + "AutoUnwrapUV")
)

fn BakeDiffuseAndAlpha obj size pName=
(
	pngio.setAlpha true;
	--Clear all render elements 
	obj.iNodeBakeProperties.removeAllBakeElements() 
	--Preparing the Bake Elements:
	be1 = VRayDiffuseFilterMap() --instance of the bake element class
	be1.outputSzX = be1.outputSzY = size --set the size of the baked map --specifythe full file path, name and type:
	be1.fileType = (pName + "texture.png")
	--be1.fileName = filenameFromPath be1.fileType
	be1.filterOn = true --enable filtering
	be1.backgroundColor = (color 127.5 127.5 127.5)
	--be1.shadowsOn = false --disable shadows
	--be1.lightingOn = false --disable lighting
	be1.enabled = true --enable baking
	obj.INodeBakeProperties.addBakeElement be1 --add first element
	--obj.INodeBakeProperties.addBakeElement be2 --add second element
	obj.INodeBakeProperties.bakeEnabled = true --enabling baking
	obj.INodeBakeProperties.bakeChannel = 2 --channel to bake
	obj.INodeBakeProperties.nDilations = 2 --expand the texturea bit
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

fn BackTexture pName bitmapSize=
(
	for OBJ in Geometry do(
		
		BakeDiffuseAndAlpha OBJ bitmapSize pName
	)
	print ("Finished " + "BackTexture")
	return (pName + "texture.png")
)

fn ExportMeshs pName zipFilename=
(
	local theClasses = exporterPlugin.classes
	local _objIdx = findItem theClasses ObjExp
	local objPath = pName + "\\OBJ\\export.obj"
	-- with *.OBJ export dialog
	--exportFile (pName) using:theClasses[_objIdx]
	-- without *.OBJ export dialog
	exportFile objPath #noprompt selectedOnly:off using:theClasses[_objIdx]
	print ("Finished " + "ExportObjects")
	return objPath
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
	print ("Finished " + "AttachMeshs")
)

fn ChangeToVray =
(
	for r in 1 to (rendererClass.classes).count where matchPattern ((rendererClass.classes)[r] as string) pattern:"V_Ray_Adv*" do
	(
		Renderers.Production = RendererClass.Classes[r]()
	)
	
	renderers.current.output_on = false
	renderers.current.imageSampler_type = 3
	renderers.current.progressive_maxSamples = 4
	renderers.current.gi_on = false
	renderers.current.options_displacement = false
)

fn ExportESS pName zipFilename=
(
	max select all
	local essPath = pName + "\\ESS\\export.ess"
	renderers.current = Elara_Renderer()
	renderers.current.UseRelativePath = false
	renderers.current.ExportSelection = true
	if renderers.current.export_ess(essPath) then print "Exported ESS."
	--renderers.current = V_Ray_Adv_3_60_03()
	ChangeToVray()
	
	local erPath = pName + "\\ElaraSDK\\bin\\er.exe"
	local packagePath = pName + "\\ESS\\export_ess.zip"
	HiddenDOSCommand (erPath + " -make_package " + essPath + " " + packagePath)
	print ("Finished " + "ExportESS")
	return packagePath
)

fn ReadMaxScene pName = 
(
	resetMaxFile #noprompt
	units.SystemScale = 1
	units.displaytype = #metric
	units.MetricType = #Millimeters
	units.SystemType = #Millimeters
	setVRaySilentMode()
	
	loadMaxFile pName useFileUnits:false quiet:true allowPrompts:false missingExtFilesAction:#abort missingDLLsAction:#abort missingXRefsAction:#abort
	
	print ("Finished " + "ReadMaxScene")
)

fn UploadESSAndObj essPack objPath texPath topTexPath x y z errorID taskID=
(
	local uplocalCommand = "UploadESSAndObj "
	uplocalCommand += (essPack + " ")
	uplocalCommand += (objPath + " ")
	uplocalCommand += (texPath + " ")
	uplocalCommand += (topTexPath + " ")
	uplocalCommand += ((x as string) + " ")
	uplocalCommand += ((y as string) + " ")
	uplocalCommand += ((z as string) + " ")
	uplocalCommand += ((errorID as string) + " ")
	uplocalCommand += (taskID as string)
	
	DOSCommand uplocalCommand
	print ("Finished " + "UploadESSAndObj")
	resetMaxFile #noprompt
)

fn CheckScene taskID =
(
	print ("Checking Scene")
	max select all
	explodeGroup selection
	local helperObjs = #()
	for o in objects where superclassof o != GeometryClass do
	(
		append helperObjs o
	)
	delete helperObjs
	if Geometry.count > 100 then
	(
		UploadESSAndObj "0" "0" "0" "0" 0 0 0 2 taskID
		return false
	)
	
	local numfaces = 0
	for OBJ in Geometry do(
		numfaces += OBJ.mesh.numfaces
	)
	
	if numfaces > 2000000 then
	(
		UploadESSAndObj "0" "0" "0" "0" 0 0 0 6 taskID
		return false
	)
	
	for i in sceneMaterials do 
	(
		if (i.category != #standard) and (i.category != #VRay) and (i.category != #2D) then
		(
			UploadESSAndObj "0" "0" "0" "0" 0 0 0 8 taskID
			return false
		)
	)
	
	return true
)

fn GetSceneSize =
(
	local myBox = Box3()
	for obj in Geometry do
	(
		local tMin = (nodeLocalBoundingBox obj)[1]
		local tMax = (nodeLocalBoundingBox obj)[2]
		expandToInclude myBox tMin
		expandToInclude myBox tMax
	)
	return [myBox.max.x - myBox.min.x, myBox.max.y - myBox.min.y, myBox.max.z - myBox.min.z]
)

fn AlignScene topPngPath =
(
	max select all
	bigGroup = group selection
	bigGroup.pos = [0, 0, 0]
	
	local mySize = GetSceneSize()
	local myCam = Freecamera()
	myCam.pos = [0, 0, 3000]
	myCam.fov = 53.13
	myCam.orthoProjection = true
	myCam.baseObject.targetDistance = mySize.x
	
	renderWidth = mySize.x
	renderHeight = mySize.y
	
	ChangeToVray()
	pngio.setAlpha true;
	render camera:myCam rendertype:#normal vfb:off progressBar:true outputSize:[mySize.x,mySize.y] outputfile:topPngPath
	print ("Finished " + "AlignScene")	
	return mySize
	
)

fn CallMeshTool maxScenename meshToolPath zipPath taskID=
(
	try
	(
		local zipFilename = getFilenameFile zipPath
		if zipPath == "" then
		(
			zipFilename = "export"
		)
		
		ReadMaxScene maxScenename
		
		if CheckScene taskID == false then return false
		
		local top_view_png = meshToolPath + "/MAP/top_view.png"
		local sceneSize = AlignScene top_view_png
		local essPack = ExportESS meshToolPath zipFilename
		OptimizeMesh 4000
		AttachMeshs()
		AutoUnwrapUV()
		local texPath = BackTexture (meshToolPath + "/OBJ/") 1024
		local objPath = ExportMeshs meshToolPath zipFilename
		UploadESSAndObj essPack objPath texPath top_view_png sceneSize.x sceneSize.y sceneSize.z 0 taskID
		--UploadESSAndObj "" "" "" top_view_png sceneSize.x sceneSize.y sceneSize.z 0 taskID
		--copyFile objPack ("E:/OBJS/" + (filenameFromPath objPack))
	)
	catch
	(
		print "Find Error!"
		print getCurrentException()
	)
	
)