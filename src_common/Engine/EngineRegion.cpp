#include <ctype.h>
#include "compiler.h"
#include "EngineRegion.h"
#include "translate.h"			// For COUN_....


static DomainRegionMap		DomainList[] = {
	"ad",	COUN_ANDORRA,				EUROPE,			
	"ae",	COUN_UNITED_ARAB_EMIRATES,	ASIA,			
	"af",	COUN_AFGHANISTAN,			ASIA,			
	"ag",	COUN_ANTIGUA,				NAMERICA,			
	"ai",	COUN_ANGUILLA,				NAMERICA,			
	"al",	COUN_ALBANIA,				EUROPE,			
	"am",	COUN_ARMENIA,				ASIA,			
	"an",	COUN_NETHERLANDS,			EUROPE,			
	"ao",	COUN_ANGOLA,				AFRICA,			
	"aq",	COUN_ANTARCTICA,			ANTARTICA,			
	"ar",	COUN_ARGENTINA,				SAMERICA,			
	"arpa",	COUN_ADV_PROJ_RES_AGENCY,	NAMERICA,			
	"as",	COUN_AMERICAN_SAMOA,		OCEANIA,			
	"at",	COUN_AUSTRIA,				EUROPE,			
	"au",	COUN_AUSTRALIA,				OCEANIA,			
	"aw",	COUN_ARUBA,					NAMERICA,			
	"az",	COUN_AZERBAIJAN,			ASIA,			
	"ba",	COUN_BOSNIA_HERZ,			EUROPE,			
	"bb",	COUN_BARBADOS,				NAMERICA,			
	"bd",	COUN_BANGLADESH,			ASIA,			
	"be",	COUN_BELGIUM,				EUROPE,			
	"bf",	COUN_BURKINA_FASO,			AFRICA,			
	"bg",	COUN_BULGARIA,				EUROPE,			
	"bh",	COUN_BAHRAIN,				ASIA,			
	"bi",	COUN_BURUNDI,				AFRICA,			
	"bj",	COUN_BENIN,					AFRICA,			
	"bm",	COUN_BERMUDA,				NAMERICA,			
	"bn",	COUN_BRUNEI,				ASIA,			
	"bo",	COUN_BOLIVIA,				SAMERICA,			
	"br",	COUN_BRAZIL,				SAMERICA,			
	"bs",	COUN_BAHAMAS,				NAMERICA,			
	"bt",	COUN_BHUTAN,				ASIA,			
	"bv",	COUN_BOUVET,				EUROPE,			
	"bw",	COUN_BOTSWANA,				AFRICA,			
	"by",	COUN_BELARUS,				EUROPE,			
	"by",	COUN_BYELORUSSIAN,			EUROPE,			
	"bz",	COUN_BELIZE,				NAMERICA,			
	"ca",	COUN_CANADA,				NAMERICA,			
	"cc",	COUN_COCOS_ISLANDS,			ASIA,			
	"cd",	COUN_CONGO,					AFRICA,			
	"cf",	COUN_CENTRAL_AFRICA,		AFRICA,			
	"cg",	COUN_CONGO,					AFRICA,			
	"ch",	COUN_SWITSERLAND,			EUROPE,			
	"ci",	COUN_COTE_D_IVOIRE,			AFRICA,			
	"ck",	COUN_COOK_ISLANDS,			OCEANIA,			
	"cl",	COUN_CHILE,					SAMERICA,			
	"cm",	COUN_CAMEROON,				AFRICA,			
	"cn",	COUN_CHINA,					ASIA,			
	"co",	COUN_COLOMBIA,				SAMERICA,			
	"com",	COUN_US_COMMERCIAL,			NAMERICA,			
	"cr",	COUN_COSTA_RICA,			NAMERICA,			
	"cs",	COUN_CZECHOSLAVAKIA,		EUROPE,			
	"cu",	COUN_CUBA,					NAMERICA,			
	"cv",	COUN_CAPE_VERDA,			AFRICA,			
	"cx",	COUN_CHRISMAS_ISLAND,		OCEANIA,			
	"cy",	COUN_CYPRUS,				ASIA,			
	"cz",	COUN_CZECH_REPUBLIC,		EUROPE,			
	"de",	COUN_GERMANY,				EUROPE,			
	"dj",	COUN_DJIBOUTI,				AFRICA,			
	"dk",	COUN_DENMARK,				EUROPE,			
	"dm",	COUN_DOMINICA,				NAMERICA,			
	"do",	COUN_DOMINICAN_REP,			NAMERICA,			
	"dz",	COUN_ALGERIA,				AFRICA,			
	"ec",	COUN_ECUADOR,				SAMERICA,			
	"edu",	COUN_US_EDUCATIONAL,		NAMERICA,			
	"ee",	COUN_ESTONIA,				EUROPE,			
	"eg",	COUN_EGYPT,					AFRICA,			
	"eh",	COUN_WESTERN_SAHARA,		AFRICA,			
	"er",	COUN_ERITREA,				AFRICA,			
	"es",	COUN_SPAIN,					EUROPE,			
	"et",	COUN_ETHIOPIA,				AFRICA,			
	"fi",	COUN_FINLAND,				EUROPE,			
	"fj",	COUN_FIJI,					OCEANIA,			
	"fk",	COUN_FALKLAND_ISLANDS,		SAMERICA,			
	"fm",	COUN_MICRONESIA,			OCEANIA,			
	"fo",	COUN_FAROS_ISLANDS,			EUROPE,			
	"fr",	COUN_FRANCE,				EUROPE,			
	"fx",	COUN_FRANCE_METRO,			EUROPE,			
	"ga",	COUN_GABON,					AFRICA,			
	"gb",	COUN_UNITED_KINGDOM,		EUROPE,			
	"gd",	COUN_GRENADA,				NAMERICA,			
	"ge",	COUN_GEORGIA,				ASIA,			
	"gf",	COUN_FRENCH_GUIANA,			SAMERICA,			
	"gh",	COUN_GHANA,					AFRICA,			
	"gi",	COUN_GIBRALTAR,				EUROPE,			
	"gl",	COUN_GREENLAND,				NAMERICA,			
	"gm",	COUN_GAMBIA,				AFRICA,			
	"gn",	COUN_GUINEA,				AFRICA,			
	"gov",	COUN_US_GOVT,				NAMERICA,			
	"gp",	COUN_GUADELOUPE,			NAMERICA,			
	"gq",	COUN_EQUATORIAL_GUINEA,		AFRICA,			
	"gr",	COUN_GREEK,					EUROPE,			
	"gs",	COUN_SOUTH_GEORGIA,			SAMERICA,			
	"gt",	COUN_GUATEMALA,				NAMERICA,			
	"gu",	COUN_GUAM,					OCEANIA,			
	"gw",	COUN_GUINEA_BISSAU,			AFRICA,			
	"gy",	COUN_GUYANA,				SAMERICA,			
	"hk",	COUN_HONG_KONG,				ASIA,			
	"hm",	COUN_HEARD_MAC_ISLANDS,		AFRICA,			
	"hn",	COUN_HONDURUS,				NAMERICA,			
	"hr",	COUN_CROATIA,				EUROPE,			
	"ht",	COUN_HAITI,					NAMERICA,			
	"hu",	COUN_HUNGARY,				EUROPE,			
	"id",	COUN_INDONESIA,				ASIA,			
	"ie",	COUN_IRELAND,				EUROPE,			
	"il",	COUN_ISRAEL,				ASIA,			
	"in",	COUN_INDIA,					ASIA,			
	"int",	COUN_INTERNATIONAL,			UNKNOWN,			
	"io",	COUN_BRITISH_INDIAN_TERR,	UNKNOWN,			
	"iq",	COUN_IRAQ,					ASIA,			
	"ir",	COUN_IRAN,					ASIA,			
	"is",	COUN_ICELAND,				EUROPE,			
	"it",	COUN_ITALY,					EUROPE,			
	"jm",	COUN_JAMAICA,				NAMERICA,			
	"jo",	COUN_JORDAN,				ASIA,			
	"jp",	COUN_JAPAN,					ASIA,			
	"ke",	COUN_KENYA,					AFRICA,			
	"kg",	COUN_KYRGYZSTAN,			ASIA,			
	"kh",	COUN_CAMBODIA,				ASIA,			
	"ki",	COUN_KIRIBATI,				OCEANIA,			
	"km",	COUN_COMOROS,				AFRICA,			
	"kn",	COUN_SAINT_KITTS_NEVIS,		NAMERICA,			
	"kp",	COUN_KOREA,					ASIA,			
	"kr",	COUN_KOREA,					ASIA,			
	"kw",	COUN_KUWAIT,				ASIA,			
	"ky",	COUN_CAYMAN_ISLANDS,		NAMERICA,			
	"kz",	COUN_KAZAKHSTAN,			ASIA,			
	"la",	COUN_LAOS,					ASIA,			
	"lb",	COUN_LEBANON,				ASIA,			
	"lc",	COUN_SAINT_LUCIA,			NAMERICA,			
	"li",	COUN_LIECHTENSTEIN,			EUROPE,			
	"lk",	COUN_SRI_LANKA,				ASIA,			
	"lr",	COUN_LIBERIA,				AFRICA,			
	"ls",	COUN_LESOTHO,				AFRICA,			
	"lt",	COUN_LITHUANIA,				EUROPE,			
	"lu",	COUN_LUXEMBOURG,			EUROPE,			
	"lv",	COUN_LATVIA,				EUROPE,			
	"ly",	COUN_LIBYA,					AFRICA,			
	"ma",	COUN_MOROCCO,				AFRICA,			
	"mc",	COUN_MONACO,				EUROPE,			
	"md",	COUN_MOLDOVA,				EUROPE,			
	"mg",	COUN_MADAGASCAR,			AFRICA,			
	"mh",	COUN_MARSHALL_ISLAND,		OCEANIA,			
	"mil",	COUN_US_MILITARY,			NAMERICA,			
	"mk",	COUN_MACEDONIA,				EUROPE,			
	"ml",	COUN_MALI,					AFRICA,			
	"mm",	COUN_MYANMAR,				ASIA,			
	"mn",	COUN_MONGOLIA,				ASIA,			
	"mo",	COUN_MACAU,					ASIA,			
	"mp",	COUN_NORTHERN_MARIANA_IS,	OCEANIA,			
	"mq",	COUN_MARTINIQUE,			NAMERICA,			
	"mr",	COUN_MAURITANIA,			AFRICA,			
	"ms",	COUN_MONTSERRAT,			NAMERICA,			
	"mt",	COUN_MALTA,					EUROPE,			
	"mu",	COUN_MAURITIUS,				AFRICA,			
	"mv",	COUN_MALDIVES,				ASIA,			
	"mw",	COUN_MALAWI,				AFRICA,			
	"mx",	COUN_MEXICO,				NAMERICA,			
	"my",	COUN_MALYSIA,				ASIA,			
	"mz",	COUN_MOZAMBIQUE,			AFRICA,			
	"na",	COUN_NAMIBIA,				AFRICA,			
	"nato",	COUN_NATO,					EUROPE,			
	"nc",	COUN_NEW_CALEDONIA,			OCEANIA,			
	"ne",	COUN_NIGER,					AFRICA,			
	"net",	COUN_NETWORK,				NAMERICA,			
	"nf",	COUN_NORFORK_ISLAND,		OCEANIA,			
	"ng",	COUN_NIGERIA,				AFRICA,			
	"ni",	COUN_NICARAGUA,				NAMERICA,			
	"nl",	COUN_NETEHRLANDS,			EUROPE,			
	"no",	COUN_NORWAY,				EUROPE,			
	"np",	COUN_NEPAL,					ASIA,			
	"nr",	COUN_NAURU,					OCEANIA,			
	"nt",	COUN_NEUTRAL_ZONE,			UNKNOWN,			
	"nu",	COUN_NIUE,					OCEANIA,			
	"nz",	COUN_NEW_ZEALAND,			OCEANIA,			
	"om",	COUN_OMAN,					ASIA,			
	"org",	COUN_US_ORGANISATION,		NAMERICA,			
	"pa",	COUN_PANAMA,				NAMERICA,			
	"pe",	COUN_PERU,					SAMERICA,			
	"pf",	COUN_FRENCH_POLYNESIA,		OCEANIA,			
	"pg",	COUN_PAPUA_NEW_GUINEA,		OCEANIA,			
	"ph",	COUN_PHILIPPINES,			ASIA,			
	"pk",	COUN_PAKISTAN,				ASIA,			
	"pl",	COUN_POLAND,				EUROPE,			
	"pm",	COUN_ST_PIERRE_MIQUELON,	NAMERICA,			
	"pn",	COUN_PITCAIRN,				OCEANIA,			
	"pr",	COUN_PUERTO_RICO,			NAMERICA,			
	"pt",	COUN_PORTUGAL,				EUROPE,			
	"pw",	COUN_PALAU,					OCEANIA,			
	"py",	COUN_PARAGUAY,				SAMERICA,			
	"qa",	COUN_QATAR,					ASIA,			
	"re",	COUN_REUNION,				AFRICA,			
	"ro",	COUN_ROMANIA,				EUROPE,			
	"ru",	COUN_RUSSIA,				EUROPE,			
	"rw",	COUN_RWANDA,				AFRICA,			
	"sa",	COUN_SAUDI_ARABIA,			ASIA,			
	"sb",	COUN_SOLOMON_ISLANDS,		OCEANIA,			
	"sc",	COUN_SEYCHELLES,			AFRICA,			
	"sd",	COUN_SUDAN,					AFRICA,			
	"se",	COUN_SWEDEN,				EUROPE,			
	"sg",	COUN_SINGAPORE,				ASIA,			
	"sh",	COUN_ST_HELENA,				AFRICA,			
	"si",	COUN_SOLENIA,				EUROPE,			
	"sj",	COUN_SVALBARD,				EUROPE,			
	"sk",	COUN_SLOVAKIA,				EUROPE,			
	"sl",	COUN_SIERRA_LEONE,			AFRICA,			
	"sm",	COUN_SAN_MARINO,			EUROPE,			
	"sn",	COUN_SENEGAL,				AFRICA,			
	"so",	COUN_SOMALIA,				AFRICA,			
	"sr",	COUN_SURINAME,				SAMERICA,			
	"st",	COUN_SAO_TOME_PRINCIPE,		AFRICA,			
	"su",	COUN_SOVIET_UNION,			ASIA,			
	"sv",	COUN_EL_SALVADOR,			NAMERICA,			
	"sy",	COUN_SYRIA,					ASIA,			
	"sz",	COUN_SWAZILAND,				AFRICA,			
	"tc",	COUN_TURKS_CAICOS_ISLANDS,	NAMERICA,			
	"td",	COUN_CHAD,					AFRICA,			
	"tf",	COUN_FRENCH_SOUTH_TERR,		AFRICA,			
	"tg",	COUN_TOGO,					AFRICA,			
	"th",	COUN_THAILAND,				ASIA,			
	"tj",	COUN_TAJIKISTAN,			ASIA,			
	"tk",	COUN_TOKELAU,				OCEANIA,			
	"tm",	COUN_TURKMENISTAN,			ASIA,			
	"tn",	COUN_TUNISIA,				AFRICA,			
	"to",	COUN_TONGA,					OCEANIA,			
	"tp",	COUN_EAST_TIMOR,			OCEANIA,			
	"tr",	COUN_TURKEY,				ASIA,			
	"tt",	COUN_TRINIDAD_AND_TOBAGO,	SAMERICA,			
	"tv",	COUN_TUVALU,				OCEANIA,			
	"tw",	COUN_TAIWAN,				ASIA,			
	"tz",	COUN_TANZANIA,				AFRICA,			
	"ua",	COUN_UKRAINE,				EUROPE,			
	"ug",	COUN_UGANDA,				AFRICA,			
	"uk",	COUN_UNITED_KINGDOM,		EUROPE,			
	"um",	COUN_US_MINOR_ISLANDS,		NAMERICA,			
	"us",	COUN_USA,					NAMERICA,			
	"uy",	COUN_URUGUAY,				SAMERICA,			
	"uz",	COUN_UZBEKISTAN,			ASIA,			
	"va",	COUN_VATICAN_CITY,			EUROPE,			
	"vc",	COUN_SAINT_VINCENT_GRENADINES,NAMERICA,			
	"ve",	COUN_VENEZUELA,				SAMERICA,			
	"vg",	COUN_VIRGIN_ISLANDS_BR,		NAMERICA,			
	"vi",	COUN_VIRGIN_ISLANDS_US,		NAMERICA,			
	"vn",	COUN_VIETNAM,				ASIA,			
	"vu",	COUN_VANUATU,				OCEANIA,			
	"wf",	COUN_WALLIS_FUTUNA_ISLANDS,	OCEANIA,			
	"ws",	COUN_SAMOA,					OCEANIA,			
	"ye",	COUN_YEMEN,					ASIA,			
	"yt",	COUN_MAYOTTE,				AFRICA,			
	"yu",	COUN_YUGOSLAVIA,			EUROPE,			
	"za",	COUN_SOUTH_AFRICA,			AFRICA,			
	"zm",	COUN_ZAMBIA,				AFRICA,			
	"zr",	COUN_ZAIRE,					AFRICA,			
	"zw",	COUN_ZIMBABWE,				AFRICA,


	"biz",	COUN_BUSINESSES,			NAMERICA,			
	"name",	COUN_INDIVIDUALS,			NAMERICA,			
	"muse",	COUN_MUSEUMS,				NAMERICA,			
	"pro",	COUN_PROFESSIONSALS,		NAMERICA,			
	"aero",	COUN_AVIATION,				NAMERICA,			
	"coop",	COUN_COOPERATIVES,			NAMERICA,			
	"info",	COUN_GNERALINFO,			NAMERICA,			

	"",0,0
};

/*
-----------------------------------------

From : http://www.norid.no/domreg.html




gTLDs 
.biz Business Organizations 
.com Commercial 
.edu Educational 
.gov US Government 
.info Open TLD 
.int International Organizations 
.mil US Dept of Defense 
.name Personal 
.net Networks 
.org Organizations 
 .ac Ascension Island 
.ad Andorra 
.ae United Arab Emirates 
.af Afghanistan 
.ag Antigua and Barbuda 
.ai Anguilla 
.al Albania 
.am Armenia 
.an Netherlands Antilles 
.ao Angola 
.aq Antarctica 
.ar Argentina 
.as American Samoa 
.at Austria 
.au Australia 
.aw Aruba 
.az Azerbaijan 
 .ba Bosnia and Herzegowina 
.bb Barbados 
.bd Bangladesh 
.be Belgium 
.bf Burkina Faso 
.bg Bulgaria 
.bh Bahrain 
.bi Burundi 
.bj Benin 
.bm Bermuda 
.bn Brunei Darussalam 
.bo Bolivia 
.br Brazil 
.bs Bahamas 
.bt Bhutan 
.bv Bouvet Island 
.bw Botswana 
.by Belarus 
.bz Belize 
 .ca Canada 
.cc Cocos (Keeling) Islands 
.cd Congo, Democratic republic of the (former Zaire) 
.cf Central African Republic 
.cg Congo, Republic of 
.ch Switzerland 
.ci Côte d'Ivoire 
.ck Cook Islands 
.cl Chile 
.cm Cameroon 
.cn China 
.co Colombia 
.cr Costa Rica 
.cs Czechoslovakia (former - non-existing) 
.cu Cuba 
.cv Cape Verde 
.cx Christmas Island 
.cy Cyprus 
.cz Czech Republic 
 

--------------------------------------------------------------------------------
 
.de Germany 
.dj Djibouti 
.dk Denmark 
.dm Dominica 
.do Dominican Republic 
.dz Algeria 
 .ec Ecuador 
.ee Estonia 
.eg Egypt 
.eh Western Sahara 
.er Eritrea 
.es Spain 
.et Ethiopia 
 .fi Finland 
.fj Fiji 
.fk Falkland Islands 
.fm Micronesia 
.fo Faroe Islands 
.fr France 
 .ga Gabon 
.gb United Kingdom 
.gd Grenada 
.ge Georgia 
.gf French Guiana 
.gg Guernsey 
.gh Ghana 
.gi Gibraltar 
.gl Greenland 
.gm Gambia 
.gn Guinea 
.gp Guadeloupe 
.gq Equatorial Guinea 
.gr Greece 
.gs South Georgia and the South Sandwich Islands 
.gt Guatemala 
.gu Guam 
.gw Guinea-Bissau 
.gy Guyana 
 

--------------------------------------------------------------------------------
 
.hk Hong Kong 
.hm Heard and McDonald Islands 
.hn Honduras 
.hr Croatia 
.ht Haiti 
.hu Hungary 
 .id Indonesia 
.ie Ireland 
.il Israel 
.im Isle of Man 
.in India 
.io British Indian Ocean Territory 
.iq Iraq 
.ir Iran 
.is Iceland 
.it Italy 
 .je Jersey 
.jm Jamaica 
.jo Jordan 
.jp Japan 
 .ke Kenya 
.kg Kyrgystan 
.kh Cambodia 
.ki Kiribati 
.km Comoros 
.kn Saint Kitts and Nevis 
.kp Korea, Democratic People's Republic of 
.kr Korea, Republic of 
.kw Kuwait 
.ky Cayman Islands 
.kz Kazakhstan 
 

--------------------------------------------------------------------------------
 
.la Lao People's Democratic Republic 
.lb Lebanon 
.lc Saint Lucia 
.li Liechtenstein 
.lk Sri Lanka 
.lr Liberia 
.ls Lesotho 
.lt Lithuania 
.lu Luxembourg 
.lv Latvia 
.ly Libyan Arab Jamahiriya 
 .ma Morocco 
.mc Monaco 
.md Moldova 
.mg Madagascar 
.mh Marshall Islands 
.mk Macedonia 
.ml Mali 
.mm Myanmar 
.mn Mongolia 
.mo Macau 
.mp Northern Mariana Islands 
.mq Martinique 
.mr Mauritania 
.ms Montserrat 
.mt Malta 
.mu Mauritius 
.mv Maldives 
.mw Malawi 
.mx Mexico 
.my Malaysia 
.mz Mozambique 
 .na Namibia 
.nc New Caledonia 
.ne Niger 
.nf Norfolk Island 
.ng Nigeria 
.ni Nicaragua 
.nl The Netherlands 
.no Norway 
.np Nepal 
.nr Nauru 
.nu Niue 
.nz New Zealand 
 .om Oman 
 

--------------------------------------------------------------------------------
 
.pa Panama 
.pe Peru 
.pf French Polynesia 
.pg Papua New Guinea 
.ph Philippines 
.pk Pakistan 
.pl Poland 
.pm St. Pierre and Miquelon 
.pn Pitcairn 
.pr Puerto Rico 
.ps Palestine 
.pt Portugal 
.pw Palau 
.py Paraguay 
 .qa Qatar 
 .re Reunion 
.ro Romania 
.ru Russia 
.rw Rwanda 
 .sa Saudi Arabia 
.sb Solomon Islands 
.sc Seychelles 
.sd Sudan 
.se Sweden 
.sg Singapore 
.sh St. Helena 
.si Slovenia 
.sj Svalbard and Jan Mayen Islands 
.sk Slovakia 
.sl Sierra Leone 
.sm San Marino 
.sn Senegal 
.so Somalia 
.sr Surinam 
.st Sao Tome and Principe 
.su USSR (former) 
.sv El Salvador 
.sy Syrian Arab Republic 
.sz Swaziland 
 

--------------------------------------------------------------------------------
 
.tc The Turks & Caicos Islands 
.td Chad 
.tf French Southern Territories 
.tg Togo 
.th Thailand 
.tj Tajikistan 
.tk Tokelau 
.tm Turkmenistan 
.tn Tunisia 
.to Tonga 
.tp East Timor 
.tr Turkey 
.tt Trinidad and Tobago 
.tv Tuvalu 
.tw Taiwan 
.tz Tanzania 
 .ua Ukraine 
.ug Uganda 
.uk United Kingdom 
.um United States Minor Outlying Islands 
.us United States 
.uy Uruguay 
.uz Uzbekistan 
 .va Holy See (Vatican City State) 
.vc Saint Vincent and the Grenadines 
.ve Venezuela 
.vg Virgin Islands British 
.vi Virgin Islands U.S 
.vn Vietnam 
.vu Vanuatu 
 .wf Wallis and Futuna Islands 
.ws Samoa 
 

--------------------------------------------------------------------------------
 
  
 .ye Yemen 
.yt Mayotte 
.yu Yugoslavia 
 .za South Africa 
.zm Zambia 
.zr Zaire (non-existent, see Congo) 
.zw Zimbabwe 

*/

/* 
NEW TLD Domain Names.

.biz - businesses
.name - individuals
.museum - museums
.pro - professionals
.aero - aviation
.coop - cooperatives
.info - general information


These are to be added to the Countries/World region lists
and the default organizations list too. They would all be
referenced as NorthAmerica since 99% of their registar is made
there and served from there.

Also they should be added to the TLD detector in Domains
report, where they should be treated in the same mannor as
.com .net .edu .mil etc...


New Domains Table.
------------------

	"biz",	COUN_BUSINESSES,			NAMERICA,			
	"name",	COUN_INDIVIDUALS,			NAMERICA,			
	"muse",	COUN_MUSEUMS,				NAMERICA,			
	"pro",	COUN_PROFESSIONSALS,		NAMERICA,			
	"aero",	COUN_AVIATION,				NAMERICA,			
	"coop",	COUN_COOPERATIVES,			NAMERICA,			
	"info",	COUN_GNERALINFO,			NAMERICA,			

*/





static	DomainRegionMapPtr 		regionPtr[26][28];

//Lookup Domain from an 2D array of indexed pointers
//for domains with more than 2 letters use the spare index of [26]
//optimised from original binary search algorithm for direct access
//initLookupGrid must be called at startup
char *LookupCountryRegion( char *code, long *region )
{
	DomainRegionMapPtr	ptr;
	short index1,index2;
	
	if ( !code ) return 0;
	if (!code[1]) return 0;

	index1=tolower(code[0])-'a';
	index2=tolower(code[1])-'a';
	if (code[2]!=0) {
		index2=26;
		if (code[3]!=0)
			index2=27;
	}
	
	if (index1 < 0 || index1 >26)
		return 0;
	if (index2 < 0 || index2 >27)
		return 0;
		
	ptr = regionPtr[index1][index2];
	if (ptr != NULL)
		*region = (ptr->fRegion);
	else
		*region = 0;

	
	ptr = regionPtr[index1][index2];
	if (ptr != NULL)
		return TranslateID( ptr->fCountryId );
	else
		return 0;
}



char *LookupDomain(char *code)
{
	DomainRegionMapPtr	ptr;
	short index1,index2;
	
	if ( !code ) return 0;
	if (!code[1]) return 0;

	index1=tolower(code[0])-'a';
	index2=tolower(code[1])-'a';
	if (code[2]!=0) {
		index2=26;
		if (code[3]!=0)
			index2=27;
	}
	
	if (index1 < 0 || index1 >26)
		return ((char *)0);
	if (index2 < 0 || index2 >27)
		return ((char *)0);
		
	ptr = regionPtr[index1][index2];
	if (ptr != NULL)
		return TranslateID( ptr->fCountryId );
	else
		return ((char *)0);
}

long LookupRegion(char *code)
{
	DomainRegionMapPtr	ptr;
	short index1,index2;

	if ( !code ) return 0;
	if (!code[1]) return 0;

	index1=tolower(code[0])-'a';
	index2=tolower(code[1])-'a';
	if (code[2]!=0) {
		index2=26;
		if (code[3]!=0)
			index2=27;
	}

	if (index1 < 0 || index1 >26)
		return 0;
	if (index2 < 0 || index2 >27)
		return 0;

	ptr = regionPtr[index1][index2];
	if (ptr != NULL)
		return (ptr->fRegion);
	else
		return 0;
} 

//create lookup grid for DomainList
void initLookupGrid(void)
{
	for( size_t x(0); DomainList[x].fCountryId; ++x )         
	{
		char *code, a, b;

		code = DomainList[x].fCode;

		a=tolower(code[0])-'a';
		b=tolower(code[1])-'a';
		if (code[2]!=0) {
			b=26;
			if (code[3]!=0)
				b=27;
		}

		regionPtr[a][b]=&DomainList[x];
	}
}

