
@ifndef(DEFAULT_MODEL_TYPE)(@define DEFAULT_MODEL_TYPE 0)  // DEFAULT_MODEL_TYPE

@* if you change Model Type, be sure to update the enum in actor.hp and in levelcon:level.hp
	TYPEENTRYINT32(Model Type,,0,6,DEFAULT_MODEL_TYPE,"Box|Mesh|Scarecrow|None|Light|Matte|Emitter",SHOW_AS_DROPMENU,"")
@*	TYPEENTRYINT32(Render Type,,0,2,0,"Actual|Wireframe|Collision Box",SHOW_AS_DROPMENU,"")


	TYPEENTRYINT32(Animation Mailbox,, 0, 4999, 1, "", SHOW_AS_NUMBER)
	TYPEENTRYINT32(Visibility Mailbox,, 0, 4999, DEFAULT_VISIBILITY, "", SHOW_AS_NUMBER)
                                                                  
                                                                  
                                                                                
	TYPEENTRYINT32(Matte Type,,0,2,1,"None|Color|Image",SHOW_AS_RADIOBUTTONS,"What kind of matte to use","ModelType == 5")

	GROUP_START(Matte Color,,"ModelType == 5 && MatteType==1")
		TYPEENTRYCOLOR(Background Color,Background,0,,"ModelType == 5 && MatteType==1")
		TYPEENTRYCOLOR(Debug Background Color,Debug Background,7829367,,"ModelType == 5 && MatteType==1")
	GROUP_STOP()

	GROUP_START(Matte Image Specification,,"ModelType == 5 && MatteType==2")
		TYPEENTRYFIXED32(Distance,,FIXED32(0),FIXED32(8000),FIXED32(8000),,,"ModelType == 5 && MatteType==2")
		TYPEENTRYFILENAME(Tiles,,BITMAP_FILESPEC,,"ModelType == 5 && MatteType==2")
		TYPEENTRYFILENAME(Map,,MAP_FILESPEC,,"ModelType == 5 && MatteType==2")
@*		TYPEENTRYBOOLEAN(Stretch Image,,0,SHOW_AS_CHECKBOX,,"ModelType == 5 && MatteType==2")
		TYPEENTRYFIXED32(X Rotation Scale,,FIXED32(-320),FIXED32(320),FIXED32(1),,,"ModelType == 5 && MatteType==2")
		TYPEENTRYFIXED32(Y Rotation Scale,,FIXED32(-240),FIXED32(240),FIXED32(1),,,"ModelType == 5 && MatteType==2")
	GROUP_STOP()

	GROUP_START(Emitter)
		TYPEENTRYINT32(generationType,Type,0,1,0,"Pulse|Continuous",SHOW_AS_DROPMENU,"","ModelType == 6")
		TYPEENTRYFIXED32(period,Period,FIXED32(0.01),FIXED32(5000),FIXED32(0.15),,,"ModelType == 6")
		TYPEENTRYFIXED32(timeDelay,Delay,FIXED32(0),FIXED32(5000),FIXED32(0),,,"ModelType == 6")

		TYPEENTRYFIXED32(yArc,YArc,FIXED32(0.01),FIXED32(1.0),FIXED32(1),,,"ModelType == 6")
		TYPEENTRYFIXED32(zArc,ZArc,FIXED32(0.01),FIXED32(1.0),FIXED32(1),,,"ModelType == 6")
		TYPEENTRYFIXED32(initialSphereRadius,Initial Radius,FIXED32(0.01),FIXED32(50),FIXED32(0.1),,,"ModelType == 6")
		TYPEENTRYFIXED32(sphereExpansionRate,Expansion Rate,FIXED32(0.01),FIXED32(50),FIXED32(0.1),,,"ModelType == 6")
		TYPEENTRYINT32(nParticles,# Particles,0,500,1,,,,"ModelType == 6")
	GROUP_STOP()

	GROUP_START(Particles)
		TYPEENTRYFIXED32(initialVelocity,Initial Velocity,FIXED32(0.01),FIXED32(10),FIXED32(0.3),,,"ModelType == 6")
		TYPEENTRYFIXED32(initialAngleA,initial A,FIXED32(0),FIXED32(1),FIXED32(0),,,"ModelType == 6")
		TYPEENTRYFIXED32(initialAngleB,initial B,FIXED32(0),FIXED32(1),FIXED32(0),,,"ModelType == 6")
		TYPEENTRYFIXED32(initialAngleC,initial C,FIXED32(0),FIXED32(1),FIXED32(0),,,"ModelType == 6")
		TYPEENTRYFIXED32(deltaAngleA,delta A,FIXED32(0),FIXED32(1),FIXED32(0),,,"ModelType == 6")
		TYPEENTRYFIXED32(deltaAngleB,delta B,FIXED32(0),FIXED32(1),FIXED32(0),,,"ModelType == 6")
		TYPEENTRYFIXED32(deltaAngleC,delta C,FIXED32(0),FIXED32(1),FIXED32(0),,,"ModelType == 6")
		TYPEENTRYFIXED32(lifeTime,C,FIXED32(0.01),FIXED32(10),FIXED32(2),,,"ModelType == 6")
		TYPEENTRYFIXED32(initialAlpha,Inital Alpha,FIXED32(0),FIXED32(1),FIXED32(1),,,"ModelType == 6")
		TYPEENTRYFIXED32(alphaDecrement,Alpha Decrement,FIXED32(0),FIXED32(1),FIXED32(0.05),,,"ModelType == 6")
	GROUP_STOP()

	GROUP_START(Force 1)
		TYPEENTRYINT32(Type,Type,0,3,0,"None|Constant|Random|Radial",SHOW_AS_DROPMENU,"")
		TYPEENTRYFIXED32(magnitude,Magnitude,FIXED32(0.01),FIXED32(10),FIXED32(0.3))
		TYPEENTRYFIXED32(vectorX,VectorX,FIXED32(0.0),FIXED32(10),FIXED32(0))
		TYPEENTRYFIXED32(vectorY,VectorY,FIXED32(0.0),FIXED32(10),FIXED32(0))
		TYPEENTRYFIXED32(vectorZ,VectorZ,FIXED32(0.0),FIXED32(10),FIXED32(0))
	GROUP_STOP()
	GROUP_START(Force 2)
		TYPEENTRYINT32(forceType,Type,0,3,0,"None|Constant|Random|Radial",SHOW_AS_DROPMENU,"")
		TYPEENTRYFIXED32(magnitude2,Magnitude,FIXED32(0.01),FIXED32(10),FIXED32(0.3))
		TYPEENTRYFIXED32(vectorX2,VectorX,FIXED32(0.0),FIXED32(10),FIXED32(0))
		TYPEENTRYFIXED32(vectorY2,VectorY,FIXED32(0.0),FIXED32(10),FIXED32(0))
		TYPEENTRYFIXED32(vectorZ2,VectorZ,FIXED32(0.0),FIXED32(10),FIXED32(0))
	GROUP_STOP()

	LEVELCONFLAGENDCOMMON
	PROPERTY_SHEET_FOOTER

@*============================================================================
