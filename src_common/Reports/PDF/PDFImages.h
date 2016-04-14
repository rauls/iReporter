
#ifndef PDFIMAGES_H
#define PDFIMAGES_H

#include <string>
#include <stdio.h>

class PDFImage
{
public:
	PDFImage()
	{ 
		imageDataBytes = 0;
		len = 0;
	} 
	~PDFImage()
	{ 
		if (imageDataBytes)
			delete[] imageDataBytes;
		imageDataBytes = 0;
		len = 0;
	}
	std::string Image() { return imageData; }
	void SetImageData( char* data, int lenP, int widthP, int heightP, int bitsPerComponentP )
	{ 
		//imageDataBytes = new char[lenP];
		//memcpy( imageDataBytes, data, lenP );
		imageDataBytes = data;
		len = lenP;
		width = widthP;
		height = heightP;
		bitsPerComponent = bitsPerComponentP;
	}
	void GetImageData( char** buf, int &lenP )
	{
		*buf = imageDataBytes;
		lenP = len;
		imageDataBytes = 0;
		len = 0;
	}

	int width;
	int height;
	int bitsPerComponent;
	char *imageDataBytes;
	int len;
	std::string imageData;
};

class PDFImageAppBanner : public PDFImage
{
public:
//	PDFImageAppBanner();

//PDFImageAppBanner::PDFImageAppBanner()
PDFImageAppBanner()
{
	imageData.reserve( 8*1024 );
	imageData =  "J,g]g3$]7K#D>EP:q1$o*=mro@So+\\<\\5,H7Uo<*jE<[.O@Wn[3@'nb-^757;Rp>H\r";
	imageData += ">q_R=AlC^cenm@9:1^AI[-E'(TMT<$3[GP8q)9(Cl=r17SPQ1`C/m7h\\6`u$$Peq!\r";
	imageData += "j`\\=EYZPJ_1\\sqJr[OaM[OYpth/Bf-?`?NIYmgE;hI:2#p%J?6^d,)inRNF@I_rmS\r";
	imageData += "QjcY!(Oq\"FYRfmg'.&-#_\")c_p1=Jlb/+0tlWbFiI>!?IcO^:f3IXA([J#L6lYS\"u\r";
	imageData += "[@[_a0&M'b6u;?8%Ls*Z`Pc8n&57NL,Ep.5E#Tes$NJTUEo@uXoG[\\AJj(9mW\"qp+\r";
	imageData += "Psfp]?CsuZJoUK1I7FhjNq'm^7p\\tKn)sV$YjSi.iujEir5Q:M99dL[XP6)+1CVa6\r";
	imageData += "cGE6P*eP9064S12brF^8__.`Elo5EJ'%;`Ar)AOpk>CZu+D8gYCD]N$XF%p%duaYj\r";
	imageData += "N+7nj0hg>:!'r\"(T#lP(4(/K5DSri72$bu4G(=KT%,en'PIhI?%eP`#bQq\\?G=s%Z\r";
	imageData += "k\"7,G:n\"3i`]V,I5g>f(\"X%b@&C&<P$lK=+D6$pU]<IeRlGCV@(G<dl1J\\i1N>`$6\r";
	imageData += "U!)LM.7DX5/:nt=H+*&V6I:r5M4o0)/^tL0??\"?Y\"EssdYTa<PcLM\\Ak<ZGnCI,Q`\r";
	imageData += "\"Xb^\"85f7,TMH7d%*Z-g2$V6;;hJAKNXZS\"`G.\\$(.JG$PS]6e:oYuL!FVo`M7!X[\r";
	imageData += "BIgKE><3m.2.jW(CW),W12P26InA2U)(2*G'WgK+[Q4X%]<%5g?W*gi(NN75kl)BR\r";
	imageData += "&GQ6(\"n_ka.Y2LuRWtCCB]:c$(EO-e<FF4fOlV8gHTOajFGgF\\ot*.H:_I=>G3$1Y\r";
	imageData += "_0dBrM?^E$i)g28XRp*-LKS:^=FXI5qspNj6XU%FJ-Q?d-!MMu1gZaY=bM'nB:.AO\r";
	imageData += "RsIu+34!F<eN1bc94!!W\\Uu4s??`ItkiW4-:CD\\oa8ma;ZMF_q+g0sCP7js'*O.ih\r";
	imageData += "F\\&U,6Zk^BKn,;IX(!6Vh6%kfn6Re4r'H<AZ]&d!B'h?)&k\"TUh<+okH9<E=3npLu\r";
	imageData += "UM0uOj4`u#^]E2N9XeM5\\3l<=cSro0X\\p_n!Ng/B6T>o[MgTd#)[W6;H6ikc2i+3p\r";
	imageData += "Ne`0e:2B6+n$!6C<9^LMoBPZ(9qPu`Tl2\"\"]Pgt:JIVcX?h?`WCZ-bafq\\T_L7E`E\r";
	imageData += "#I[r+HD5^._#B0\\*X79O9a8p4:dIp44GXW@$BGZ!PNP3a8/R;&/#C[](^OajK1L1J\r";
	imageData += "]t$6!g9V&7NZ^c\\Jspk+hj-Vt1k[9kW5q#(fN:jIJM\\2ZZE+^CE7P4i<ZqP6f+X9;\r";
	imageData += "!$A@[B#F1iJUeX&4V;E-_D_,p9RuY]CqZ/g7!`J[$%P2Mlq4>`lD4q>rZFoU/:]X-\r";
	imageData += "n!Ft!;GY8[`F@IG(;l1>TTF?0AO$r\\^k\\\"L\"],:D_Iu;UCiJq>S2PC!l]^&g6`;S?\r";
	imageData += "62#E5bfSj#OL2a'oXt)-;aW<5-/5-c36.&+8NU`&KL]`)>8UP>Jt[!1NtLOgJTtN/\r";
	imageData += "TtPEF\"\\90ZG[Osrr&-t]DGB2s#96CU8%/4QcD)8pD9#NA`gK[7*LR6uW:g]$!&W$F\r";
	imageData += "i8Pl^!/=\"O%1X_1>S83f:q[\\o(^f&M$F2*s\"7UdR*8:r7iN^'l&ns1*ace;1SA1QE\r";
	imageData += "gEn8rE?K=6>UUn3Y#(TQ8Ak:U]E-ar#V<$3;OE#c\\iW38Mj/5J7<\"\"%[,Zbj1('ct\r";
	imageData += "J4Lfk%M#.L&(<S('.679W+%lQ:%ZQEPH]dRZ`p2^.6/MJr.mm(P%aLX#03-E$j[I`\r";
	imageData += "P'$>oYmH<[Bimtpid)pY@,D,($ks->li8b$X]Gg?#U%Kr$(_@TgSFXq95LbL-n!_a\r";
	imageData += "a=O'fLNr[2A#3DX]X4I\\p2\\adN:Gn<6NuG-I#b_('1F]aT!S2M!@+O&6Z<Eg`!ge@\r";
	imageData += "i_rXo;\"8qQ^\"/9Kmb49plupo+N*#;4?m1Fh+O?q4J-7h>\"@^/:<t2YY+KBc@5a=O+\r";
	imageData += "%:\\%#MEXPO!\"eeR5_Ysg+JKEA\"eeIQ#0\"FnJrit5!0DZ>d7abB..RQ$Ep\"'MjM#M/\r";
	imageData += "S.7INf:g>3?AL,2U9n(<;.f0q#XH'R<CI4Ma@L*_A^.ob&`XA5/Tt_^&Lui!H-</i\r";
	imageData += "'1.d(!a$_!NF:NO/2PGG,r5^gU\"VS(OGF^[OKT#-&8Ma/=VnT]%HBbn.j^;c`c`P!\r";
	imageData += "j,sg0KWVpAa/^?B;ApkoE(t=Mlm@?-eu1o3;?-e>E8^ec(Vj7m*FZHidW,I<ZA&_-\r";
	imageData += "fo7)C)h)pE\\IN$1AE_;@<^\\*n$u!2]^KAo5Rbld1#sqKJ-EJc:JWNJ7^]7+q=ccY\"\r";
	imageData += "_$%qkDKl^`NXdm`AC<^DURMo%RKu)u+rC<6Tu0BfLe`1>.7CN&_BZ\"LnFpuj;5h$?\r";
	imageData += "=\\?Qd'F@.^;;%d3B1NU4)!0*N%mgk\\b;eq<HOae^8-h=Si[Gr[Uah^_pfYBP51sk2\r";
	imageData += "G9gZ>AnjR=YfCc?.$KQ/e/JWDjfEr,N9ZHtX[ojq&La3o%qq^5`BNH\"G1^mHpn6jG\r";
	imageData += "%=eN'>$E;+.:Z<iJrDoH!4O-W\\AB(M=omf%U^P$^\"ism]1%&\"s@`J(;4m0L\"JYZ+.\r";
	imageData += ")E99)k/Du&K.<-SWN(T0o8>#Y=CT37iW*Fed?_L>!D^kPUOr)q8$hO+#P5i*.N[IX\r";
	imageData += "i(,LIZ,$HSH_Q%q7n5TiJ2DM)`!s!6NWqK7&6.6I`3hoNUkhnT5u;iegi6@Ri*-^Z\r";
	imageData += "q69BURMs(GJaPi-9*m64gdDZmmiB`bG@<$KKcm2[rR\\f.nOP+K!6Q.1j1pOlE%V;R\r";
	imageData += "VsllGb'@>Ai&i*6&jS:'$:4Ru8+7r!!OPaMcMg%YBU88kpWiis\"lR&o8b9a<deVd(\r";
	imageData += "/&Q:t'V;MNQesniKrCdISod6nnH&^(+<2K?\"B#4Cr$B'*1&.Fb;NK`u2aL7O*?e7/\r";
	imageData += "#naBAJ7-_6!&-s'!P1/n0n9SjFEE3;r$BB+$m4B6IgHY6!<t5RrSqPd$LnHF^_dmh\r";
	imageData += "P5[b+)@t%2!JhV3f_j^?!J.g#!J'qg8/D;mnkWT]cS5M4/H]rO'S2np@>GbOJFX7&\r";
	imageData += "n1Z-UY71*m\"S3.1n':.ci?\\\"K@.3OIn?=\\E&\"jIc=A?R2+\"O5\"M2I*SA5j[^+2G;o\r";
	imageData += "Y3>:]gBhcZ#(U%P!)s-;3X.jVmr%q-+LD><>m`hY$]qk#Ye^_j.1@oH$ZO2rJ6W:t\r";
	imageData += "^&t#W%C!YXYf@1^!X6B($KnVt9M4b+pBM85&\"s(/cKOigE:Y_$(]fp?D]&$gC@RBI\r";
	imageData += "\"0c>*Dj1Ufr:_<Pq*?;\"UAP!e<$/pP'$jDW07NRI!qVHSo`7SoUB'tJ)A5@6!'p\\X\r";
	imageData += "JD(c98JMbL'#9\\+d5Vm03>1dW#pK3Z6#$M=q@_c8*?`T'TqDjHq@(^\\)Zd/]!r<5l\r";
	imageData += "o_Y>n%&3e?Dqt8p!#bjAqk;IU6#-GXTGmp\\!0>>E+gD.])Zd47*0Ha.d23SVT^kkZ\r";
	imageData += "\"`Q0u:g\"C(8/:[h++\\2=iIN+)*Ya0@++\\DROC7mF.1hOomVlkPn<3AsCEBYa\"lTLl\r";
	imageData += "TY@_)?F$MaXU?bC,shX:Rj,$,,[i6M+;b4-e-3XjmguT-:pg_IB+4:U%PZ!OEbPkW\r";
	imageData += "[K^bl%PZihnnnY>=p[H]cle\\^J<\\!To^@nBmdU0!0:=#GAFe\\T!?mEm0-ME5/KV1`\r";
	imageData += "o.NB$&RGH78Jb/d\"s@@]+^Xe\\4S@-uo8f42YtF]8M?0B6/Fe96iCOj&rWs`_r]s*O\r";
	imageData += "0:EuuY4muN'$ldK%r<$q&0J^d)R&!8@@e\"Hq?@P%)0+(XTp?T-g^=.p)k^RH!2'W!\r";
	imageData += "N;_)h+2BH,JKPej3#<1G)8Hk,d@C/tT,GJo*H04D+ikV.Y4=kt)kh6qP.Lm:<@^il\r";
	imageData += "(%347!P0*K:)'C\\(_QL&o9A`!#nTr?+jqM<;CrH$V)o1J'&[(j_877K.%&0*VP%K8\r";
	imageData += "d/NKoi?W=!!l-r.+C'5sY6r(r#uJ)X;J@Jao04qZ-^&fbEe+YiG[)@s..W3&E:WHq\r";
	imageData += "?LG+k4Ye5c1$o&2-Lh,&(sjQgOnA-O8G=>*'4;aOdbsdoAFsei!oX4W;'bh^8G05_\r";
	imageData += "pJam%iG1Hi8GPN6&2AP7P\\0\\&B_NW4'5pGrZ$ZW$fENKA&d%nM0s(8fYReR9r_L30\r";
	imageData += "iNOu/L`',M1U#XS!8n.aM#TaT(_?]I0BsL[qZ#JW(r;1K:X0m[]/%md*KV'q[+@>^\r";
	imageData += "8/ZAB!qG_G%ke$g,6VFH&C[*<eG/MVECNm)3g;K/ngb=AB*L%g3tCWF!7p%C8g$m;\r";
	imageData += "amafeW1F$mnHP.L,/Db6+A4%:=U>_2mpOm6d*2t3Ad2X-5BU6s;uHu@`=;=m-ID!B\r";
	imageData += "dZ4m\"bpcO`.<;+2OSo`2\\O'5jnY'^g1bgLWDD/1l&l3WD@JgEdHj2;`\"-5\\K_-Z^J\r";
	imageData += "VY;odq9_henQHJ[?TF/o0<m/FO#ll`JGLp>!'o-2c^=KG\"%.U#(%7\"^P`5T[g_07R\r";
	imageData += "!'qh):WiacTHDNIs&$cKiVWln,QX.F!n.\\__:'8eSe<\\Q(9`T&'aFY&i=PP-(m3E*\r";
	imageData += "[+$$.Er?R`3Ag=p:c0D9)b(uWnGu/te12Y\"?5<0<!b$X''_+8+\\KoO7\"n5A\"P,0Ff\r";
	imageData += "e.<cUGu?Pem9[XZ<X(f&$+?BMDWNQ[RKK3p#sbm)NooeOlW#jB$^,*(no6m;^H[qe\r";
	imageData += "$'r,%dWn+NEslsI5r7+=n7qCAJI+.T6?S1i5qN5C.;r4MAQ&['08hM[0JDA<&jT;/\r";
	imageData += "UA>>'=;*?B0<^d4Dj9sV5VJcn\"7Rl9dJ$np<u-?Y2<\"K/5hdiN?X2\"Q(de'.J[$0!\r";
	imageData += "L^sRc14c,>1nl)@1Do%%*093'o1*d$r!e4-*#BQ<@;u']*uNoT\">nWu<7<V0f`A<,\r";
	imageData += "n0+,Ydt'JWLCpfe35Jr)EWR#b@3-5h(]dMM;>3O[M*#db%o]TCeIr,Z)$;hFCG+`^\r";
	imageData += "1VV\"?0S(Yh;S[_h@+HFHp?m&?&0S/&TOnM7kmFZm%<YH1np+58>?-qj\"8,l!?orr5\r";
	imageData += "`!1D/$+@a'&AnrUm3rgqD\".$t[6#PYCO_YB=AJqeeW04.$M(Y$KoZ`Vdt;43m5m\"2\r";
	imageData += "mPO;\"0UmH2Mf)?>)A[*Ko8<(DnHElo:D)mfe1MA\",S[Hl*>Hp>@3Q>N?N^D%1'*1K\r";
	imageData += "n^B9NdLq*aMU1rdTcb40bS\\/E&i2<5PDsok`rRn*8n!uZ:dGuM4\\[XK6F7mq&bCeH\r";
	imageData += ".j('o3OJZ[GIJ\\NS/;9+'j1KK'o-Eu@k$!\",#KG3ZDDH(Je0O-*%Ra*CSk'jrfNi[\r";
	imageData += "4`L2aJFA!'@m7?%.2K)Sns=mumgQY(C6l\\efGb7o*J*h`%!KHOa0?'\"i=(ggD'56]\r";
	imageData += "TOX7+]*oN@5*^[((?KFq>mClM'_@eu2Ka=eM^pJ?7Mpo8(fqoL(D(R>6;Hj%:c]F/\r";
	imageData += "9cm7<O]`>2?jRs:-Up\"&*O2/C<Wu3KYSlDa)-Q2r']/p)8W[&p9YlQ,(23hg+$$fi\r";
	imageData += "T\"*ZPNtmZEgNi669[gIIcrC[>3:[)i3MB_=Tn/iH#ru)':ULLk2K\\G2+bWi1+n0+8\r";
	imageData += "<hUthEpc2g)!`XmCFn(L-)P'W;bloq@0-gY:R#8A5esdloh+WPgHP5s!ju*0R9bBR\r";
	imageData += "*=Ouq%J00!EfZR\\_^=\"l$UBFP@$h9d3X-42KDAACYkLGJ*GQ+Y=L+tWont;</ND'*\r";
	imageData += "+4:B`):aFAnVG]`.WAN43q%AJ.j+MTY#&9'P.MluFp!9t+Km5g[pPhQ?l5@H1GPGn\r";
	imageData += "e>G2_6NI_;&fCiU0L?ZYT(-6.(:7QSE>BDEl5LWT!?4P%o.-pY@9@XeU+eNc0b'Q%\r";
	imageData += "@?@1K'Y6%:_,M/c8W]WEXT:3[Os-WH(O`Mu&Mb5;&GqkJj:FW>9u/MXb%[_a_`[TQ\r";
	imageData += "q@-mA5T0d.nl*?,%A;*`p>56u<=CRW\"(51;;uJ)3?^>ZIWOC2h\\1%kiGsE.V$(A&b\r";
	imageData += ">.VOkkCaQ^I5#`1P.@\".)2-E/L_1F+\\$$hdM86^kDYd#.2H?qV/OGmP\"/0'go^/kd\r";
	imageData += "\"0hFB9p7+K+eP?nZa'?ASsPEWR,sE**Z,hN8p`M9FU'o(=c3,E`50#&>]7!#M2O<a\r";
	imageData += "'&\"jRo7]P/pTqC*6?AS1=t@M%0hg&]9\\X^!@8n'^431Slo3-i\\f?pDJ9-O*Kb7<YB\r";
	imageData += "@4!%(9IHl+\\q\"?j:@s.!5RQH<JC^n1H:Lj5el1>N%\"M^f?n7MSdqej6<UeDMq0rio\r";
	imageData += "BEhk\"-\"'s[I0H\"-oae?5muV)l&K_0TS/";
	imageData += "/^''G:5`GYf.L[tlaX[@6^(2H.[:*NiH;\r";
	imageData += "O4O13GL:_=pTuh=A2>e,=KiZK?_L_C\">AG'&4=R&!Z9]r)<%9qS='t:=ReLZ`u?75\r";
	imageData += "g'mLN.e)nZ[@+\\uGJru0YBg&0[c*RIpUCp@*YlLj'fKpNqgX;u\\U`=Kn5HYgiA89;\r";
	imageData += "54<pE6-t\\t78Oi[4G`3Ual$]/A[1,G3:7pC%<.?q&?F]WIt7?MWKgWi*YF_0rIR(q\r";
	imageData += "K,2f?]aMPPBl!c1#lpeJ?oeC=1VYHpXEWK&04,#@s(*i<=W0YTH95`nA](4H7TO6E\r";
	imageData += "[?XNKrQ:Qp;6_&AQc5a5\\![sK68'pshH>\"8PK,E49fa\"01\"R8IclG&PfAW#93UND^\r";
	imageData += "Q(R$/UE!dM)=,rq8[ONnTCadmfOK7*M`B=tattRbe!]S(DAN(,(`g/KgnTtX<I2<X\r";
	imageData += "'&CV<EUO+[)#9Y\\I67>LP\"H5C3#LT/'Yt8#l;kd`7jKaM%pf(X?2C.C[1;!rJ\\:a(\r";
	imageData += "N2o1Y4p2U)Jc7N-SHjb0OfMZV$pHNrS+$P'2l_Bh^Rng)s'bUcAbLUfp:kJ<s)Ict\r";
	imageData += "Fn^B2pV2.Nl7;t0ef5i,b(u*m81PJuJbae%eBZnGs.CY\"+;k-:VG]N],>Nk5U\\fPN\r";
	imageData += "Z,$Pai@#=iXeo-`\"c_L<^X<!/SdXKe*GhYeOFCFW?iY\\arEnskkI)=;rXH[R&qSUI\r";
	imageData += "!WXl#MZZb*\"<J-]KgOa[fQ9;#&W&p8O]\\ts0c>s?j,r!dIR,->8E>q:1kjLu6\\#KE\r";
	imageData += "F*Onlm?>cVP?G9LEB0RbkEFTkr]66Y&t&*ke/!U,V,HQBg3#TQ;5lq=#^3IR@oKL1\r";
	imageData += "l]p2sI_fHSWJ[?.22]dq[9Ds`gN7QLhRD;.QWplSnThfSn!Df%rrI\\e@.9Og2BNNf\r";
	imageData += "`FAA)gibn*Oi_/)R9[1-,lm~>\r";
}
};

class PDFAppBannerASCII85Decode
{
public:
	//PDFAppBannerASCII85Decode();
//PDFAppBannerASCII85Decode::PDFAppBannerASCII85Decode()
PDFAppBannerASCII85Decode()
{
	data.reserve( 2*1024 );
	data =  "s8W*$$4[%G'bCof!\"p.U()m5l-4(Y?*#opu*\\.440H_J\\&0WM',VoWR3\".8q'eD<[\r";
	data += "/3#14/H@Ch-SA^S/m\\Q39.)B#-nes]0JP780IS\\e!Am=o1h7+U;C4>:.5tus2c1q_\r";
	data += "D(.J^3B9/.4Cfd%E[a.m!'sg86$=;9;^atX6:+!?6u4)9HN6L(1dt4A7Uu`oL*K3>\r";
	data += "!(gc*8Tb80MBYZG7nQMs98!FASOH4O!DR>9:7<.tM@*.P(/Kf<:La`.TGp^d7o<Mg\r";
	data += ";,Ig$T+t7a(/f)O;fi[PSde\\_&lk&-<MC@3S1%ET-!,$e=FD/kSLmr`9NYn-=Bc2:\r";
	data += "PQP*E1KnfC>FPe%R0R#p$!ir<?)AfOV(5i;8R6Hq?][EXF@]ef?smA/@<F54I7%It\r";
	data += "!+UrjA\"?b5E<BdH2eA<XAsOrZX=Itd3,\":bBkd]bV>tJ52/TAoDOWM$YQ\\sR2/n^;\r";
	data += "E3a(pHi\\5hEH,suEh,mEQ998q!d17<G-XheWW@FQ\"FI'`H_be2K1LC!I!g;AIC^hT\r";
	data += "V$MUTJ:N.MK\"Y'8]E!Z&0nl`!KrP3/\\7UJe>)BqEM3.dq``7nr?&ZQ!Nlk+W\\W2]1\r";
	data += "FHNd-P0IX&`0-:H'9gK@Q-95odYF6n\".I[tR`]`AaV(,p$_>lPT%FICfc$I6St;Me\r";
	data += "T@k0Ug'=>''qs>-U\"TpLg)QjBIA4t$UXgK^f.<`R,Gj!KV;,%HdQFD_L8N1lVPbuC\r";
	data += "fL)Rn2Q>LpWSiQ2conDc7B53:Wnp.Ei/FbcPH;i\"Xl30[hcd/A<Nb;YYIUj?Y>qjb\r";
	data += "<O(OfZ/8flij,kq@^P$'[,dU^f5%hUD7SbL]&oa\"fs&-2HG8Ke^?E>H^OEu]I_b6H\r";
	data += "^tlfA^k9MeKYm#T_r&#Bc,7H:^r4?NaQ^+Uj0m.k\\'*dUc-=S^e'lgsaOf\\4h;-N9\r";
	data += "m+C:aioB+RjmhHuo],<1mdBN0nb2DBq=F4Mo(MqRq>1-lz~>\r";
}
	std::string Data() { return data; }
protected:
	std::string data;

};



class PDFImageTest : public PDFImage
{
public:
//	PDFImageTest();
//PDFImageTest::PDFImageTest()
PDFImageTest()
{
	imageData.reserve( 256 );
	imageData += "003B00 003B00 002700 002700 002480 002480 0E4940 0E4940 114920 114920 14B220 14B220 3CB650 3CB650\r";
	imageData += "75FE88 75FE88 17FF8C 17FF8C 175F14 175F14 1C07E2 1C07E2 3803C4 3803C4 703182 703182 F8EDFC F8EDFC\r";
	imageData += "B2BBC2 B2BBC2 BB6F84 BB6F84 31BFC2 31BFC2 18EA3C 18EA3C 0E3E00 0E3E00 07FC00 07FC00 03F800 03F800\r";
	imageData += "1E1800 1E1800 1FF800 1FF800\r";
}
};


/*#ifndef WORD
#define WORD unsigned short
#endif
#ifndef DWORD
#define DWORD unsigned long
#endif
#ifndef LONG
#define LONG unsigned long
#endif


#ifndef BITMAPFILEHEADER
typedef struct tagBITMAPFILEHEADER { 
  WORD    bfType; 
  DWORD   bfSize; 
  WORD    bfReserved1; 
  WORD    bfReserved2; 
  DWORD   bfOffBits; 
} BITMAPFILEHEADER, *PBITMAPFILEHEADER; 
#endif

#ifndef BITMAPINFOHEADER
typedef struct tagBITMAPINFOHEADER{
  DWORD  biSize; 
  LONG   biWidth; 
  LONG   biHeight; 
  WORD   biPlanes; 
  WORD   biBitCount; 
  DWORD  biCompression; 
  DWORD  biSizeImage; 
  LONG   biXPelsPerMeter; 
  LONG   biYPelsPerMeter; 
  DWORD  biClrUsed; 
  DWORD  biClrImportant; 
} BITMAPINFOHEADER, *PBITMAPINFOHEADER; 
#endif

typedef struct tagBitmapHead
{
	BITMAPFILEHEADER bmpFileHead;
	BITMAPINFOHEADER bmpInfoHead;
} BitmapHead;

class PDFImageTest2 : public PDFImage
{
public:
//	PDFImageTest2();
//	~PDFImageTest2();
//PDFImageTest2::PDFImageTest2()
PDFImageTest2()
{
	colors = 0;
	data = 0;
	
	std::string filename = "C:/ForMeanPathFix/src_win/gui/application.bmp";
	char openmode[10];
	int bytes = 0;
	strcpy( openmode, "r" );
	fp = fopen( filename.c_str(), openmode );

	if ( fp )
	{
		bytes = fread( buf, 1, 54, fp );
		bmpFileHead.bfType = BytesToShort( &buf[0] );
		bmpFileHead.bfSize += BytesToInt( &buf[2] ); 
		bmpFileHead.bfOffBits += BytesToInt( &buf[10] );

		bmpInfoHead.biSize = BytesToInt( &buf[14] );
		bmpInfoHead.biWidth = BytesToInt( &buf[18] );
		bmpInfoHead.biHeight = BytesToInt( &buf[22] );
		bmpInfoHead.biPlanes = BytesToShort( &buf[26] );
		bmpInfoHead.biBitCount = BytesToShort( &buf[28] );
		bmpInfoHead.biCompression = BytesToInt( &buf[30] );
		bmpInfoHead.biSizeImage = BytesToInt( &buf[34] );
		bmpInfoHead.biXPelsPerMeter = BytesToInt( &buf[38] );
		bmpInfoHead.biYPelsPerMeter = BytesToInt( &buf[42] );
		bmpInfoHead.biClrUsed = BytesToInt( &buf[46] );
		bmpInfoHead.biClrImportant = BytesToInt( &buf[50] );

		colors = (void*)new char[ bmpFileHead.bfSize ];
		bytes = fread( colors, 1, bmpFileHead.bfSize, fp );

		fclose( fp );
	}

	imageData.reserve( 256 );
	imageData += "003B00 003B00 002700 002700 002480 002480 0E4940 0E4940 114920 114920 14B220 14B220 3CB650 3CB650\r";
	imageData += "75FE88 75FE88 17FF8C 17FF8C 175F14 175F14 1C07E2 1C07E2 3803C4 3803C4 703182 703182 F8EDFC F8EDFC\r";
	imageData += "B2BBC2 B2BBC2 BB6F84 BB6F84 31BFC2 31BFC2 18EA3C 18EA3C 0E3E00 0E3E00 07FC00 07FC00 03F800 03F800\r";
	imageData += "1E1800 1E1800 1FF800 1FF800\r";
}

~PDFImageTest2()
//PDFImageTest2::~PDFImageTest2()
{
	if ( data )
		delete[] data;
}
	int BytesToInt( char *buf )
	{
		int val;
		val = buf[0]; 
		val += buf[1]*256; 
		val += buf[2]*256*256; 
		val += buf[3]*256*256*256;
		return val;
	}
	short BytesToShort( char *buf )
	{
		short val;
		val = buf[0]; 
		val += buf[1]*256; 
		return val;
	}

	FILE *fp;
	BITMAPFILEHEADER bmpFileHead;
	BITMAPINFOHEADER bmpInfoHead;
//	BitmapHead bitmapHead;
	char buf[56];
	void *colors;
	void *data;
};
*/

//extern char *imageDataBuf;
class PDFImageJpeg : public PDFImage
{
public:
	PDFImageJpeg(){ imageDataBytes = 0; }
	~PDFImageJpeg(){};// if (imageDataBytes) delete[] imageDataBytes; imageDataBytes = 0; }
//	char *imageDataBytes;
};

#endif // PDFIMAGES_H
