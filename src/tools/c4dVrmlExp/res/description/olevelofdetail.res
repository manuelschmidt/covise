CONTAINER olevelofdetail
{
	NAME Olevelofdetail;
	INCLUDE Obase;

	GROUP ID_OBJECTPROPERTIES
	{
		STATICTEXT	BESCHREIBUNG { }
		GROUP
		{	
			COLUMNS 3;	
			REAL LOD_CENTER_X		{ UNIT METER; MIN 0.0;}
			REAL LOD_CENTER_Y		{ UNIT METER; MIN 0.0;}
			REAL LOD_CENTER_Z		{ UNIT METER; MIN 0.0;}
		}
		
		SEPARATOR
		{
			LINE;
		}
		
		GROUP
		{
			COLUMNS 3;
			STATICTEXT DISTANCE_CIRCLES {}
			BOOL LOD_OBJECT_CIRCLES		{}
		}
		SEPARATOR
		{
			LINE;
		}
	}
}
