@*============================================================================
@* activate.inc
@*============================================================================

@*	TYPEENTRYINT32(Activated By,, 0, 3, 1, "All|Actor|Class|Object List", SHOW_AS_RADIOBUTTONS )
	TYPEENTRYINT32(Activated By,, 0, 3, 1, "All|Actor|Class|List", SHOW_AS_RADIOBUTTONS )
	GROUP_START(_Activated By,,"ActivatedBy!=0")
		TYPEENTRYOBJREFERENCE(Activated By Actor,Actor,,"ActivatedBy==1",,,"Player01" )
		TYPEENTRYCLASSREFERENCE(Activated By Class,Class,,"ActivatedBy==2")
		TYPEENTRYXDATA_CONVERT(Activated By Object List,List,"Cave Logic Studios Object List [Activated By]|",XDATA_NOT_REQUIRED,,"ActivatedBy==3",,, XDATA_OBJECTLIST )
	GROUP_STOP()

	LEVELCONFLAGENDCOMMON
              
@*============================================================================
