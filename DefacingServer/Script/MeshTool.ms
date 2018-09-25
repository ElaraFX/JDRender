global baseObjects
global glassObjects
global metalObjects
global needBakeObject
global meshToolRunTimes

fn OptimizeMesh maxFace = 
(
	for OBJ in Geometry  where (canconvertto OBJ Editable_Mesh) do(
		InstanceMgr.MakeObjectsUnique OBJ #prompt
		local numfaces = OBJ.mesh.numfaces
		
		
		if numfaces > 256 then
		(	
			if numfaces < 8000 then
			(
				local factor = 1.0 - (numfaces / 8000.0) * 0.5
				local vp = factor * 100.0
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
			proOpt.MergePoints = on
			proOpt.PreventFlip = on
			proOpt.LockPoints = on
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
	if needBakeObject != undefined then(
		local OBJ = needBakeObject
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
	pngio.settype(#true24)
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
	if needBakeObject != undefined then(
		local OBJ = needBakeObject
		
		BakeDiffuseAndAlpha OBJ bitmapSize pName
		print ("Finished " + "BackTexture")
		return (pName + "texture.png")
	)else
	(
		print ("Finished " + "BackTexture")
		return ("none")
	)
)

fn ExportMeshs pName zipFilename=
(
	local exportSettings = objExp.getIniName()
	setINISetting exportSettings "Material" "UseMapPath" "1"
	setINISetting exportSettings "Material" "MapPath" "./"
	setINISetting exportSettings "Geometry" "FaceType" "0"
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

fn CheckMaterial mtl=
(
	classOfMtl = classof mtl
	if classOfMtl == VRayMtl then
	(
		if mtl.Refraction.value > 130 then
		(
			 --glass
			return 1
		)else(
			if mtl.Reflection.value > 130 then
			(
				if mtl.reflection_fresnel == off then
				(
					return 2
				)else
				(
					if mtl.reflection_lockIOR == off and mtl.reflection_ior < 0.2 then
					(
						return 2
					)
				)				
			)
		)			
	)
	 --base
	return 0
)

fn ColorToHex c=
(
	ret = "0x"
	h = bit.intashex c.r
	if h.count < 2 then
	(
		h = ("0"+h)
	)
	ret+= h

	h = bit.intashex c.g
	if h.count < 2 then
	(
		h = ("0"+h)
	)
	ret+= h

	h = bit.intashex c.b
	if h.count < 2 then
	(
		h = ("0"+h)
	)
	ret+= h
	ret
)

fn AttachMeshs =
(
	baseObjects = #()
	glassObjects = #()
	metalObjects = #()
	
	for OBJ in Geometry where (canconvertto OBJ Editable_Mesh) do(
		try(
			--resetxform OBJ
			convertToPoly OBJ
			if isKindOf OBJ Editable_Poly then
			(
				local mtlType = CheckMaterial OBJ.material
				if mtlType == 1 then
				(
					OBJ.name = OBJ.name + "_glass_color_" + ColorToHex(OBJ.material.Refraction)
					append glassObjects OBJ
				)else if mtlType == 2 then
				(
					OBJ.name = OBJ.name + "_metal_color_" + ColorToHex(OBJ.material.Reflection) + "_glossiness_" + (int(OBJ.material.reflection_glossiness * 100) as string)
					append metalObjects OBJ
				)else
				(
					append baseObjects OBJ
				)
			)
		) catch()
	)
	
	for i = 2 to baseObjects.count do
	(
		 polyop.attach baseObjects[1] baseObjects[i]
	)
	
	if baseObjects.count > 0 then
	(
		needBakeObject = baseObjects[1]
	)else
	(
		needBakeObject = undefined
	)
	
	print ("Finished " + "AttachMeshs")
)

fn ChangeToVray =
(
	renderers.current = Default_Scanline_Renderer()
	for r in 1 to (rendererClass.classes).count where matchPattern ((rendererClass.classes)[r] as string) pattern:"V_Ray_Adv*" do
	(
		Renderers.Production = RendererClass.Classes[r]()
	)
	
	renderers.current.output_on = false
	renderers.current.imageSampler_type = 3
	renderers.current.progressive_maxSamples = 4
	renderers.current.gi_on = false
	renderers.current.options_displacement = false
	renderers.current.options_defaultLights = 2
	
	rendOutputFilename = ""
	rendSaveFile = false
	
	local re = maxOps.GetCurRenderElementMgr() -- get the current render element manager
	re.removeallrenderelements() -- remove all renderelements
)

fn ChangeToElara =
(
	renderers.current = Elara_Renderer()
	renderers.current.MaxSamples = 4
	renderers.current.DiffuseDepth = 1
	renderers.current.SumDepth = 1
	
	rendOutputFilename = ""
	rendSaveFile = false
	
	local re = maxOps.GetCurRenderElementMgr() -- get the current render element manager
	re.removeallrenderelements() -- remove all renderelements
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
	
	try
	(
		loadMaxFile pName useFileUnits:false quiet:true allowPrompts:false missingXRefsAction:#abort --missingExtFilesAction:#abort missingDLLsAction:#abort 
	)
	catch
	(
		print "Wrong max File!"
	)
	
	print ("Finished " + "ReadMaxScene")
)

fn UploadESSAndObj essPack objPath texPath mtlPath topTexPath x y z errorID taskID=
(
	local uplocalCommand = "UploadESSAndObj "
	uplocalCommand += (essPack + " ")
	uplocalCommand += (objPath + " ")
	uplocalCommand += (texPath + " ")
	uplocalCommand += (mtlPath + " ")
	uplocalCommand += (topTexPath + " ")
	uplocalCommand += ((x as string) + " ")
	uplocalCommand += ((y as string) + " ")
	uplocalCommand += ((z as string) + " ")
	uplocalCommand += ((errorID as string) + " ")
	uplocalCommand += (taskID as string)
	
	resetMaxFile #noprompt
	DOSCommand uplocalCommand
	print ("Finished " + "UploadESSAndObj")	
	
	if meshToolRunTimes == undefined then
	(
		meshToolRunTimes = 0
	)
	
	if meshToolRunTimes > 5 then
	(
		--quitMAX quiet:true
	)else
	(
		meshToolRunTimes += 1
	)
)

fn GetSceneSize =
(
	local clearObjs = for obj in Geometry  where (canconvertto obj Editable_Mesh) collect obj
	local myBox = Box3()
	for obj in clearObjs do
	(
		local tempObj = editable_mesh name:obj.name
		convertToPoly tempObj
		polyop.attach  tempObj obj
		local tMin = (nodeLocalBoundingBox tempObj)[1]
		local tMax = (nodeLocalBoundingBox tempObj)[2]
		expandToInclude myBox tMin
		expandToInclude myBox tMax
	)
	return [myBox.max.x - myBox.min.x, myBox.max.y - myBox.min.y, myBox.max.z - myBox.min.z]
)

fn CheckScene taskID =
(
	print ("Checking Scene")
	max select all
	xrefs.deleteAllXRefs()
	
	if selection.count == 0 then
	(
		UploadESSAndObj "0" "0" "0" "0" "0" 0 0 0 9 taskID
		return false
	)
	
	explodeGroup selection
	
	local helperObjs = #()
	local hideFrozen = maxOps.hideFrozenObjects
	for o in objects where ((superclassof o != GeometryClass) or (o.isHidden) or (hideFrozen and o.isFrozen) or not (canconvertto o Editable_Mesh)) do
	(
		append helperObjs o
	)
	delete helperObjs
	
	if Geometry.count == 0 then
	(
		UploadESSAndObj "0" "0" "0" "0" "0" 0 0 0 9 taskID
		return false
	)
	
	if Geometry.count > 100 then
	(
		UploadESSAndObj "0" "0" "0" "0" "0" 0 0 0 2 taskID
		return false
	)
	
	local numfaces = 0
	for OBJ in Geometry where (canconvertto OBJ Editable_Mesh) do(
		numfaces += OBJ.mesh.numfaces
	)
	
	if numfaces > 2000000 then
	(
		UploadESSAndObj "0" "0" "0" "0" "0" 0 0 0 6 taskID
		return false
	)
	
	for i in sceneMaterials do 
	(
		if (i.category != #standard) and (i.category != #VRay) and (i.category != #2D) then
		(
			UploadESSAndObj "0" "0" "0" "0" "0" 0 0 0 8 taskID
			return false
		)
	)
	
	local sceneSize = GetSceneSize()
	
	if sceneSize.x > 15000 or sceneSize.y > 15000 or sceneSize.z > 3000 then
	(
		UploadESSAndObj "0" "0" "0" "0" "0" 0 0 0 7 taskID
		return false
	)	
	
	return sceneSize
)

fn AlignScene topPngPath sceneSize =
(
	local geoObjs = for obj in Geometry  where (canconvertto obj Editable_Mesh) collect obj
	bigGroup = group geoObjs
	bigGroup.pos = [0, 0, 0]
	
	local mySize = sceneSize
	local myCam = Freecamera()
	myCam.pos = [0, 0, 3000]
	myCam.fov = 53.13
	myCam.orthoProjection = true
	myCam.baseObject.targetDistance = mySize.x
	
	local maxEdge = 512
	if mySize.x > mySize.y then
	(
		renderWidth = maxEdge
		renderHeight = maxEdge * mySize.y / mySize.x
	)else
	(
		renderWidth = maxEdge * mySize.x / mySize.y
		renderHeight = maxEdge
	)
	
	ChangeToVray()
	--useEnvironmentMap = off
	--backgroundColor = color 255 255 255
	--local tempLight = Omnilight multiplier:0
	pngio.settype(#true24)
	pngio.setAlpha true;
	render camera:myCam rendertype:#normal vfb:off progressBar:true outputSize:[renderWidth,renderHeight] outputfile:topPngPath
	--delete tempLight
	ungroup bigGroup
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
		
		local sceneSize = CheckScene taskID
		if sceneSize == false then
		(
			return false
		)
		
		local top_view_png = meshToolPath + "/MAP/top_view.png"
		AlignScene top_view_png sceneSize
		local essPack = ExportESS meshToolPath zipFilename
		OptimizeMesh 4000
		AttachMeshs()
		AutoUnwrapUV()
		local texPath = BackTexture (meshToolPath + "/OBJ/") 1024
		local objPath = ExportMeshs meshToolPath zipFilename
		local mtlPath = meshToolPath + "\\OBJ\\export.mtl"
		
		UploadESSAndObj essPack objPath texPath mtlPath top_view_png sceneSize.x sceneSize.y sceneSize.z 0 taskID
	)
	catch
	(
		print "Find Error!"
		format "*** % ***\n" (getCurrentException())
		resetMaxFile #noprompt
		UploadESSAndObj "0" "0" "0" "0" "0" 0 0 0 10 taskID
	)
)
uiaccessor.closedialog (windows.getchildhwnd 0 "MAXScript Listener")[1]
renderSceneDialog.close()