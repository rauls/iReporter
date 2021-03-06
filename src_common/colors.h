
#define COLOR_LEN 768

 static unsigned char color[]=
 {	0xDE, 0xDE, 0xDE, 0xDA, 0xDA, 0xDA, 0xD8, 0xD8, 0xD8, 0xD5, 0xD5, 0xD5, 0xBC, 0xBC, 0xBC, 0x9C,
	0x9C, 0x9C, 0x7C, 0x7C, 0x7C, 0x66, 0x66, 0x66, 0x62, 0x62, 0x62, 0x5E, 0x5E, 0x5E, 0x5D, 0x5D,
	0x5D, 0x57, 0x57, 0x57, 0x52, 0x52, 0x52, 0x47, 0x47, 0x47, 0x3C, 0x3C, 0x3C, 0x1A, 0x1A, 0x1A,
	0xFF, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xE0, 0x00, 0x00, 0xD1, 0x00, 0x00, 0xC1, 0x00, 0x00, 0xB2,
	0x00, 0x00, 0xA3, 0x00, 0x00, 0x93, 0x00, 0x00, 0x84, 0x00, 0x00, 0x74, 0x00, 0x00, 0x65, 0x00,
	0x00, 0x56, 0x00, 0x00, 0x46, 0x00, 0x00, 0x37, 0x00, 0x00, 0x27, 0x00, 0x00, 0x18, 0x00, 0x00,
	0xFF, 0xFF, 0x00, 0xEF, 0xEF, 0x00, 0xDE, 0xDE, 0x00, 0xCE, 0xCE, 0x00, 0xBD, 0xBD, 0x00, 0xAD,
	0xAD, 0x00, 0x9C, 0x9C, 0x00, 0x8C, 0x8C, 0x00, 0x7B, 0x7B, 0x00, 0x6B, 0x6B, 0x00, 0x5A, 0x5A,
	0x00, 0x4A, 0x4A, 0x00, 0x39, 0x39, 0x00, 0x29, 0x29, 0x00, 0x18, 0x18, 0x00, 0x08, 0x08, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xEF, 0x00, 0x00, 0xDF, 0x00, 0x00, 0xCF, 0x00, 0x00, 0xBF, 0x00, 0x00,
	0xAF, 0x00, 0x00, 0x9F, 0x00, 0x00, 0x8F, 0x00, 0x00, 0x80, 0x00, 0x00, 0x70, 0x00, 0x00, 0x60,
	0x00, 0x00, 0x50, 0x00, 0x00, 0x40, 0x00, 0x00, 0x30, 0x00, 0x00, 0x20, 0x00, 0x00, 0x10, 0x00,
	0x00, 0xFF, 0xFF, 0x00, 0xEF, 0xEF, 0x00, 0xDE, 0xDE, 0x00, 0xCE, 0xCE, 0x00, 0xBE, 0xBE, 0x00,
	0xAD, 0xAD, 0x00, 0x9D, 0x9D, 0x00, 0x8D, 0x8D, 0x00, 0x7C, 0x7C, 0x00, 0x6C, 0x6C, 0x00, 0x5C,
	0x5C, 0x00, 0x4B, 0x4B, 0x00, 0x3B, 0x3B, 0x00, 0x2B, 0x2B, 0x00, 0x1A, 0x1A, 0x00, 0x0A, 0x0A,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xEF, 0x00, 0x00, 0xE0, 0x00, 0x00, 0xD0, 0x00, 0x00, 0xC0, 0x00,
	0x00, 0xB0, 0x00, 0x00, 0xA1, 0x00, 0x00, 0x91, 0x00, 0x00, 0x81, 0x00, 0x00, 0x71, 0x00, 0x00,
	0x62, 0x00, 0x00, 0x52, 0x00, 0x00, 0x42, 0x00, 0x00, 0x32, 0x00, 0x00, 0x23, 0x00, 0x00, 0x13,
	0xFC, 0x00, 0xFF, 0xED, 0x00, 0xF0, 0xDD, 0x00, 0xE0, 0xCE, 0x00, 0xD1, 0xBF, 0x00, 0xC1, 0xB0,
	0x00, 0xB2, 0xA0, 0x00, 0xA3, 0x91, 0x00, 0x93, 0x82, 0x00, 0x84, 0x73, 0x00, 0x74, 0x63, 0x00,
	0x65, 0x54, 0x00, 0x56, 0x45, 0x00, 0x46, 0x36, 0x00, 0x37, 0x26, 0x00, 0x27, 0x17, 0x00, 0x18,
	0xA6, 0xC4, 0xF1, 0x9C, 0xBB, 0xE9, 0x91, 0xB1, 0xE0, 0x87, 0xA8, 0xD8, 0x7D, 0x9E, 0xD0, 0x73,
	0x95, 0xC8, 0x68, 0x8B, 0xBF, 0x5E, 0x82, 0xB7, 0x54, 0x78, 0xAF, 0x4A, 0x6F, 0xA7, 0x3F, 0x65,
	0x9E, 0x35, 0x5C, 0x96, 0x2B, 0x52, 0x8E, 0x21, 0x49, 0x86, 0x16, 0x3F, 0x7D, 0x0C, 0x36, 0x75,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x0B, 0x00, 0xFF, 0x16, 0x00, 0xFF, 0x21, 0x00,
	0xFF, 0x2C, 0x00, 0xFF, 0x36, 0x00, 0xFF, 0x41, 0x00, 0xFF, 0x4C, 0x00, 0xFF, 0x57, 0x00, 0xFF,
	0x62, 0x00, 0xFF, 0x6D, 0x00, 0xFF, 0x78, 0x00, 0xFF, 0x83, 0x00, 0xFF, 0x8D, 0x00, 0xFF, 0x98,
	0x00, 0xFF, 0xA3, 0x00, 0xFF, 0xAE, 0x00, 0xFF, 0xB3, 0x00, 0xFF, 0xB8, 0x00, 0xFE, 0xBD, 0x00,
	0xFE, 0xC2, 0x00, 0xFE, 0xC7, 0x00, 0xFE, 0xCC, 0x00, 0xFE, 0xD1, 0x00, 0xFE, 0xD7, 0x00, 0xFD,
	0xDC, 0x00, 0xFD, 0xE1, 0x00, 0xFD, 0xE6, 0x00, 0xFD, 0xEB, 0x00, 0xFD, 0xF0, 0x00, 0xFC, 0xF5,
	0x00, 0xFC, 0xFA, 0x00, 0xFC, 0xFF, 0x00, 0xED, 0xFF, 0x00, 0xDE, 0xFF, 0x00, 0xCF, 0xFF, 0x00,
	0xC0, 0xFF, 0x00, 0xB1, 0xFF, 0x00, 0xA2, 0xFF, 0x00, 0x93, 0xFF, 0x00, 0x84, 0xFF, 0x00, 0x75,
	0xFF, 0x00, 0x66, 0xFF, 0x00, 0x57, 0xFF, 0x00, 0x48, 0xFF, 0x00, 0x39, 0xFF, 0x00, 0x2A, 0xFF,
	0x00, 0x1B, 0xFF, 0x00, 0x0C, 0xFF, 0x00, 0x10, 0xF6, 0x0F, 0x13, 0xEE, 0x1E, 0x17, 0xE5, 0x2D,
	0x1A, 0xDD, 0x3D, 0x1E, 0xD4, 0x4C, 0x21, 0xCC, 0x5B, 0x25, 0xC3, 0x6A, 0x29, 0xBB, 0x79, 0x2C,
	0xB2, 0x88, 0x30, 0xA9, 0x97, 0x33, 0xA1, 0xA6, 0x37, 0x98, 0xB6, 0x3A, 0x90, 0xC5, 0x3E, 0x87,
	0xD4, 0x41, 0x7F, 0xE3, 0x45, 0x76, 0xF2, 0x45, 0x79, 0xF2, 0x45, 0x7D, 0xF2, 0x45, 0x80, 0xF2,
	0x45, 0x83, 0xF2, 0x45, 0x87, 0xF2, 0x45, 0x8A, 0xF2, 0x45, 0x8E, 0xF2, 0x45, 0x91, 0xF2, 0x45,
	0x94, 0xF2, 0x45, 0x98, 0xF2, 0x45, 0x9B, 0xF2, 0x45, 0x9E, 0xF2, 0x45, 0xA2, 0xF2, 0x45, 0xA5,
	0xF2, 0x45, 0xA9, 0xF2, 0x45, 0xAC, 0xF2, 0x45, 0xAF, 0xF2, 0x45, 0xB3, 0xF2, 0x45, 0xB6, 0xF2,
	0x45, 0xB9, 0xF2, 0x45, 0xBD, 0xF2, 0x45, 0xC0, 0xF2, 0x45, 0xC4, 0xF2, 0x45, 0xC7, 0xF2, 0x45,
	0xCA, 0xF2, 0x45, 0xCE, 0xF2, 0x45, 0xD1, 0xF2, 0x45, 0xD4, 0xF2, 0x45, 0xD8, 0xF2, 0x45, 0xDB,
	0xF2, 0x45, 0xDF, 0xF2, 0x45, 0xE2, 0xF2, 0x45, 0xE5, 0xF2, 0x45, 0xE9, 0xF2, 0x45, 0xEC, 0xF2,
	0xF2, };

