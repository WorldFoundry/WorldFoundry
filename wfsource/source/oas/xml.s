@*========================================g======================================
@* xml.s generates an xml schema for this object type
@*==============================================================================
<?xml version="1.0" encoding="US-ASCII"?>
<xs:schema
     xmlns:xs="http://www.w3.org/2001/XMLSchema">

  <xs:annotation>
    <xs:documentation xml:lang="en">
    created from xml.s DO NOT MODIFY
    </xs:documentation>
  </xs:annotation>
                                      
<!--=========================================================================-->
<!-- created from xml.s DO NOT MODIFY                                        -->
<!--=========================================================================-->

@define IFF_S

@* get common definitions
@include types.h
@* kts what was oad.ph? 7/11/99
@* @include oad.ph

@*@define FIXED16(num)  ((short) num * 256)
@define FIXED32(num) @=f(num)

@define RADIOBUTTONNAMELEN 10

@define TAB@t

@define        SHOW_AS_N_A              0@t
@define        SHOW_AS_NUMBER           1@t                      @* numbers
@define        SHOW_AS_SLIDER           2@t                      @* numbers
@define        SHOW_AS_TOGGLE           3@t
@define        SHOW_AS_DROPMENU         4@t
@define        SHOW_AS_RADIOBUTTONS     5@t
@define        SHOW_AS_HIDDEN           6@t                      @* anything
@define        SHOW_AS_COLOR            7@t                      @* int32
@define	       SHOW_AS_CHECKBOX	        8@t
@define	       SHOW_AS_MAILBOX		9@t
@define	       SHOW_AS_COMBOBOX	        10@t
@define	       SHOW_AS_TEXTEDITOR	11@t
@define	       SHOW_AS_FILENAME  	12@t
@define	       SHOW_AS_OBJECTREFERENCE	13@t

@* maximum value of a long int
@define LONG_MAX    2147483647
@* minimum value of a long int
@define LONG_MIN    -2147483647

@define	XDATA_IGNORE "Conversion: Ignore"
@define	XDATA_COPY "Conversion: Copy"
@define	XDATA_OBJECTLIST "Conversion: ObjectList"
@define	XDATA_CONTEXTUALANIMATIONLIST "Conversion: AnimationList"
@define	XDATA_SCRIPT "Conversion: Convert"
@define	XDATA_CONVERSION_MAX "Conversion: Convert Max"


@*============================================================================
@* create array

@define TYPEHEADER(displayName,variableName=displayName) @define OASNAME variableName@t@n <xs:complexType name="@+displayName@+">@n
@define TYPEFOOTER </xs:complexType>

@define PROPERTY_SHEET_HEADER(sheetname,active=0,szEnableExpression="1",size=0)@-	<xs:complexType name="@+sheetname@+"> <!-- PROPERTY SHEET START open=active -->@n
@define PROPERTY_SHEET_FOOTER()	</xs:complexType>	<!-- PROPERTY SHEET END -->

@* only int32's are allowed until we get better structure alignment
@define TYPEENTRYINT32(elementname, displayName=elementname, min, max, def=min, buttons="", showas=SHOW_AS_NUMBER,helpstring="",szEnableExpression="1",y=-1,x=-1)@\
@-TAB		<xs:element name="@+elementname@+" type="cs:integer" default="@+def@+" >@n@\
TAB                     <xs:simpleType>@n@\
TAB                       <xs:restriction base="xs:integer">@n@\
TAB                         <xs:minInclusive value="min" />@n@\
TAB                         <xs:maxInclusive value="max" />@n@\
TAB                       </xs:restriction>@n@\
TAB                     </xs:simpleType>@n@\
TAB                     <displayname>"@+displayName@+"</displayname>@n@\
TAB                     <enumeration>@+buttons@+</enumeration>@n@\
TAB                     <help>@+helpstring@+</help>@n@\
TAB                     <enable>@+szEnableExpression@+</enable>@n@\
TAB                     <displayas>@+showas@+</displayas>@n@\
TAB		</xs:element>

@* only fixed32's are allowed until we get better structure alignment
@define TYPEENTRYFIXED32(elementname, displayName=elementname, min, max, def=min,showas=SHOW_AS_NUMBER,helpstring="",szEnableExpression="1",y=-1,x=-1)@\
@-TAB		<xs:element name="@+elementname@+" type="cs:decimal" default="@+def@+" >@n@\
TAB                     <xs:simpleType>@n@\
TAB                       <xs:restriction base="xs:integer">@n@\
TAB                         <xs:minInclusive value="min" />@n@\
TAB                         <xs:maxInclusive value="max" />@n@\
TAB                       </xs:restriction>@n@\
TAB                     </xs:simpleType>@n@\
TAB                     <displayname>"@+displayName@+"</displayname>@n@\
TAB                     <enumeration>@+buttons@+</enumeration>@n@\
TAB                     <help>@+helpstring@+</help>@n@\
TAB                     <enable>@+szEnableExpression@+</enable>@n@\
TAB                     <displayas>@+showas@+</display>@n@\
TAB		</xs:element>

@define TYPEENTRYSTRING_IGNORE(elementname,displayName=elementname,helpstring="",szEnableExpression="1",y=-1,x=-1,showas=SHOW_AS_N_A) @\
@-TAB		<xs:element name="@+elementname@+" type="cs:string">@n@\
TAB                     <displayname>"@+displayName@+"</displayname>@n@\
TAB                     <help>helpstring</help>@n@\
TAB                     <enable>szEnableExpression</enable>@n@\
TAB                     <displayas>showas</display>@n@\
TAB		</xs:element>


@define TYPEENTRYBOOLEAN(elementname,displayName=elementname, def,showas=SHOW_AS_CHECKBOX,helpstring="",szEnableExpression="1",y=-1,x=-1)@\
@-TAB		<xs:element name="@+elementname@+" type="cs:boolean" default="@+def"@+ > <!-- boolean -->@n@\
TAB                     <xs:simpleType>@n@\
TAB                       <xs:restriction base="xs:integer">@n@\
TAB                         <xs:minInclusive value="0" />@n@\
TAB                         <xs:maxInclusive value="1" />@n@\
TAB                       </xs:restriction>@n@\
TAB                     </xs:simpleType>@n@\
TAB                     <displayname>"@+displayName@+"</displayname>@n@\
TAB                     <enumeration>"False|True" </enumeration>@n@\
TAB                     <help>helpstring</help>@n@\
TAB                     <enable>szEnableExpression</enable>@n@\
TAB                     <displayas>showas</display>@n@\
TAB		</xs:element>

@define TYPEENTRYBOOLEANTOGGLE(elementname,displayName=elementname, def,showas=SHOW_AS_RADIOBUTTONS,buttons="FALSE|TRUE",helpstring="",szEnableExpression="1",y=-1,x=-1)@\
@-TAB		<xs:element name="@+elementname@+" type="cs:integer" default="@+def@+" > <!-- boolean -->@n@\
TAB                     <xs:simpleType>@n@\
TAB                       <xs:restriction base="xs:integer">@n@\
TAB                         <xs:minInclusive value="0" />@n@\
TAB                         <xs:maxInclusive value="1" />@n@\
TAB                       </xs:restriction>@n@\
TAB                     </xs:simpleType>@n@\
TAB                     <displayname>"@+displayName@+"</displayname>@n@\
TAB                     <enumeration>"False|True" </enumeration>@n@\
TAB                     <help>helpstring</help>@n@\
TAB                     <enable>szEnableExpression</enable>@n@\
TAB                     <displayas>showas</display>@n@\
TAB		</xs:element>

@define TYPEENTRYOBJREFERENCE(elementName,displayName=elementName, helpstring="",szEnableExpression="1",y=-1,x=-1,def="")@\
TAB		<xs:element name="@+elementName@+" type="cs:string" default="@+def@+" > <!-- Object Reference -->@n@\
TAB			<displayname>"@+displayName@+"</displayname>@n@\
TAB			<hint>Object Reference</hint>@n@\
TAB                     <help>helpstring</help>@n@\
TAB                     <enable>szEnableExpression</enable>@n@\
TAB                     <displayas>SHOW_AS_OBJECTREFERENCE</display>@n@\
TAB		</xs:element>

@define TYPEENTRYFILENAME(elementName,displayName=elementName,filespec="*.*",helpstring="",szEnableExpression="1",y=-1,x=-1) @\
TAB		<xs:elemen t name="@+elementName@+" type="cs:string"> <!-- filename -->@n@\
TAB			<displayname>"@+displayName@+"</displayname>@n@\
TAB			<hint>FILESPEC: filespec</hint>@n@\
TAB                     <help>helpstring</help>@n@\
TAB                     <enable>szEnableExpression</enable>@n@\
TAB                     <displayas>SHOW_AS_FILENAME</display>@n@\
TAB		</xs:element>


@define TYPEENTRYXDATA_CONVERT(elementName,displayName=elementName, chunkName,required, helpstring="",szEnableExpression="1",y=-1,x=-1, conversion=XDATA_COPY) @\
TAB		<xs:element name="@+elementName@+" type="cs:string" def=@+chunkName@+> @n@\
TAB			<displayname>"@+displayName@+"</displayname>@n@\
TAB			<hint>conversion</hint>@n@\
TAB			<required>required</required>@n@\
TAB                     <help>helpstring</help>@n@\
TAB                     <enable>szEnableExpression</enable>@n@\
TAB                     <displayas>SHOW_AS_TEXTEDITOR</display>@n@\
TAB		</xs:element>

@define TYPEENTRYCOLOR(elementName, displayName=elementName, def, helpstring="",szEnableExpression="1",y=-1,x=-1)  @\
@-TAB	        <xs:element name="@+elementName@+" type="cs:integer" default="@+def@+" >@n@\
TAB			<displayname>"@+displayName@+"</displayname>@n@\
TAB                     <xs:simpleType>@n@\
TAB                       <xs:restriction base="xs:integer">@n@\
TAB                         <xs:minInclusive value="0" />@n@\
TAB                         <xs:maxInclusive value="16777215" />@n@\
TAB                       </xs:restriction>@n@\
TAB                     </xs:simpleType>@n@\
TAB			<hint>Color</hint>@n@\
TAB                     <displayas>SHOW_AS_COLOR</display>@n@\
TAB                     <help>helpstring</help>@n@\
TAB                     <enable>szEnableExpression</enable>@n@\
TAB		</xs:element>

@define TYPEENTRYCAMERA(elementName,displayName=elementName, followObj, szEnableExpression="1") @\
@-TAB	        <xs:element name="@+elementName@+" type="cs:string" default=followObj>@n@\
TAB			<displayname>"@+displayName@+"</displayname>@n@\
TAB			<hint>Extract Camera</hint>@n@\
TAB                     <displayas>SHOW_AS_HIDDEN</display>@n@\
TAB                     <help>Indicates the camera will be updated with new values such as look at, roll, and FOV</help>@n@\
TAB                     <enable>szEnableExpression</enable>@n@\
TAB		</xs:element>

@define TYPEENTRYWAVEFORM(name,displayName=name, helpstring="", szEnableExpression="1") { BUTTON_WAVEFORM, "@+name@+", 0, 0, 0, 0, "", SHOW_AS_N_A, -1, -1, help, {XDATA_IGNORE,0,"@+displayName@+"", szEnableExpression }},

@define TYPEENTRYCLASSREFERENCE(name,displayName=name, helpstring="",szEnableExpression="1",y=-1,x=-1,def="") @\
TAB		<xs:element name=@+elementName@+ type="cs:string" default="@+def@+" > <!-- class reference -->@n@\
TAB			<displayname>"@+displayName@+"</displayname>@n@\
TAB			<hint>Class Reference</hint>@n@\
TAB                     <help>helpstring</help>@n@\
TAB                     <enable>szEnableExpression</enable>@n@\
TAB		</xs:element>

@define GROUP_START(groupname,width=0,szEnableExpression="1",y=-1,x=-1)@-TAB		<xs:complexType name="@+groupname@+"  ><!-- GROUP START -->@n@undef TAB @define TAB@t@n
@define GROUP_STOP(y=-1,x=-1)@-TAB	</xs:complexType> <!-- GROUP STOP -->@undef TAB @define TAB@t

@* entries which are only used by the level converter

@define LEVELCONFLAGSHORTCUT	@\
@-TAB		<hint>ShortCut</hint>

@define LEVELCONFLAGNOINSTANCES @\
@-TAB		<hint>No Instances</hint>

@define LEVELCONFLAGEXTRACTLIGHT @\
@-TAB		<hint>Extract Light</hint>
@define LEVELCONFLAGEXTRACTCAMERANEW

@define LEVELCONFLAGROOM @\
@-TAB		<hint>Extract Room</hint>

@define LEVELCONFLAGCOMMONBLOCK(blockname)@\
@-TAB	<xs:complexType name="@+blockname@+">@n@\
TAB     <flag>COMMONBLOCK</flag>    <!-- COMMONBLOCK -->@undef TAB @define TAB	@t@include blockname@+.inc
@define LEVELCONFLAGENDCOMMON @-TAB	</xs:complexType>	<!-- ENDCOMMON -->

<!--=======================================================================-->

@include TYPEFILE_OAS@+.oas

</xs:schema>

<!--=======================================================================-->

