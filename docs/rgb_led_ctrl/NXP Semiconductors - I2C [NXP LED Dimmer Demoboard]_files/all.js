﻿function sTopNav(){var sHtml='';sHtml+='<table id="sTopNav">';sHtml+='<tr>';sHtml+='<td><a href="/to/" onmouseover="fnStatusSet(\'http://www.nxp.com/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">Home</a></td>';sHtml+='<td class="sTopNavSpace">|</td>';sHtml+='<td><a href="/to/profile/" onmouseover="fnStatusSet(\'http://www.nxp.com/profile/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">About&nbsp;NXP</a></td>';sHtml+='<td class="sTopNavSpace">|</td>';sHtml+='<td><a href="/to/news/" onmouseover="fnStatusSet(\'http://www.nxp.com/news/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">News</a></td>';sHtml+='<td class="sTopNavSpace">|</td>';sHtml+='<td><a href="/to/infocus/" onmouseover="fnStatusSet(\'http://www.nxp.com/infocus/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">In&nbsp;Focus</a></td>';sHtml+='<td class="sTopNavSpace">|</td>';sHtml+='<td><a href="/to/jobs/" onmouseover="fnStatusSet(\'http://www.nxp.com/jobs/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">Careers</a></td>';sHtml+='<td class="sTopNavSpace">|</td>';sHtml+='<td><a href="/to/investor/" onmouseover="fnStatusSet(\'http://www.nxp.com/investor/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">Investors</a></td>';sHtml+='<td class="sTopNavSpace">|</td>';sHtml+='<td><a href="/to/support/" onmouseover="fnStatusSet(\'http://www.nxp.com/support/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">Contact</a></td>';sHtml+='<td class="sTopNavSpace">|</td>';sHtml+='<td><a href="/to/my/" onmouseover="fnStatusSet(\'http://www.nxp.com/my/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">my.NXP</a></td>';sHtml+='</tr>';sHtml+='</table>';document.write(sHtml);}function sR1C6(){var sHtml='';sHtml+='<table style="width:100%;">';sHtml+='<tr>';sHtml+='<td style="width:15px;" onclick="sLangOff(); return true;"><img src="/images/nxp/r1c6_c1.gif" alt="" /></td>';sHtml+='<td style="background:url(/images/nxp/r1c6_c2.gif) repeat-x; padding-right:3px;">';sHtml+='<div style="padding-top:10px;">';sHtml+='<table>';sHtml+='<tr>';sHtml+='<td onclick="sLangOff(); return true;"><span style="font-weight:bold; white-space:nowrap;">Select site:</span></td>';sHtml+='<td width="100%">';sHtml+='<div id="sLangDropDown">';sHtml+='<div id="sLangCurrent">English</div>';sHtml+='<div id="sLangButton" onclick="sLangToggle(); return false;"><img src="/images/nxp/dropdown.gif" alt="" /></div>';sHtml+='</div>';sHtml+='<div id="sLangSelect" style="visibility:hidden;">';sHtml+='<div id="sLangItemGlobal" onmouseover="this.style.backgroundColor=\'#cccccc\'; return true;" onmouseout="this.style.backgroundColor=\'#ffffff\'; return true;">Global site</div>';sHtml+='<div id="sLangItemEn" onmouseover="this.style.backgroundColor=\'#cccccc\'; return true;" onmouseout="this.style.backgroundColor=\'#ffffff\'; return true;" onclick="window.location.href=\'/to/\'; return false;">English</div>';sHtml+='<div id="sLangItemJp" onmouseover="this.style.backgroundColor=\'#cccccc\'; return true;" onmouseout="this.style.backgroundColor=\'#ffffff\'; return true;" onclick="window.location.href=\'/go/http://www.jp.nxp.com/\'; return false;" title="Japanese">日本語</div>';sHtml+='<div id="sLangItemKr" onmouseover="this.style.backgroundColor=\'#cccccc\'; return true;" onmouseout="this.style.backgroundColor=\'#ffffff\'; return true;" onclick="window.location.href=\'/go/http://www.kr.nxp.com/\'; return false;" title="Korean">한국어</div>';sHtml+='<div id="sLangItemCn" onmouseover="this.style.backgroundColor=\'#cccccc\'; return true;" onmouseout="this.style.backgroundColor=\'#ffffff\'; return true;" onclick="window.location.href=\'/go/http://www.scn.nxp.com/\'; return false;" title="Simplified Chinese">中文(简体)</div>';sHtml+='<div id="sLangItemTw" onmouseover="this.style.backgroundColor=\'#cccccc\'; return true;" onmouseout="this.style.backgroundColor=\'#ffffff\'; return true;" onclick="window.location.href=\'/go/http://www.tcn.nxp.com/\'; return false;" title="Traditional Chinese">中文(繁體)</div>';sHtml+='<div id="sLangItemRu" onmouseover="this.style.backgroundColor=\'#cccccc\'; return true;" onmouseout="this.style.backgroundColor=\'#ffffff\'; return true;" onclick="window.location.href=\'/go/http://www.ru.nxp.com/\'; return false;" title="Russian">Русский</div>';sHtml+='</div>';sHtml+='</td>';sHtml+='</tr>';sHtml+='</table>';sHtml+='</div>';sHtml+='<div id="sNxpSearch" onclick="sLangOff(); return true;">';sHtml+='<form action="/search/nxpsearch.cgi" method="get">';sHtml+='<table style="width:100%;">';sHtml+='<tr><td id="sNxpSearchWidth"><input id="sNxpSearchInput" onfocus="this.value=\'\'; this.style.color=\'#000000\'; return true;" type="text" name="search" maxlength="30" value="Type search here" /></td><td><input style="padding:0px;" type="image" name="Search" src="/images/nxp/b_search.gif" /></td></tr>';sHtml+='<tr><td colspan="2"><img src="/images/t.gif" alt="" height="2" width="160" /></td></tr>';sHtml+='<tr><td id="sNxpSearchRadio" colspan="2"><table><tr><td><input class="sRadio" style="margin-left:0px;" type="radio" name="where" value="part" checked="checked" /></td><td class="sLabel">Type #</td><td><input class="sRadio" type="radio" name="where" value="cross" /></td><td class="sLabel">Cross-ref</td><td><input class="sRadio" type="radio" name="where" value="site" /></td><td class="sLabel">Site</td></tr></table></td></tr>';sHtml+='</table>';sHtml+='</form>';sHtml+='</div>';sHtml+='<div style="margin-bottom:0px; padding-top:2px;" onclick="sLangOff(); return true;"><ul class="arrowbulletwhite"><li><a href="/to/search/" onmouseover="fnStatusSet(\'http://www.nxp.com/search/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;"><span style="color:#ffffff; font-weight:bold;">Advanced search</span></a></li></ul></div>';sHtml+='</td>';sHtml+='<td style="width:10px;" onclick="sLangOff(); return true;"><img src="/images/nxp/r1c6_c3.gif" alt="" /></td>';sHtml+='</tr>';sHtml+='</table>';document.write(sHtml);}function sR2C6(){var sHtml='';sHtml+='<table>';sHtml+='<tr>';sHtml+='<td rowspan="3"><img src="http://198.65.99.86/images/nxp/r2c6_c1.gif" alt="" /></td>';sHtml+='<td colspan="3" style="background:url(http://198.65.99.86/images/nxp/r2c6_r1.gif) repeat-x; height:3px;"></td>';sHtml+='<td rowspan="3"><img src="http://198.65.99.86/images/nxp/r2c6_c5.gif" alt="" /></td>';sHtml+='<td colspan="3" style="background:url(http://198.65.99.86/images/nxp/r2c6_r1.gif) repeat-x; height:3px;"></td>';sHtml+='<td rowspan="3"><img src="http://198.65.99.86/images/nxp/r2c6_c9.gif" alt="" /></td>';sHtml+='</tr>';sHtml+='<tr>';sHtml+='<td style="background:url(http://198.65.99.86/images/nxp/r2c6_r2a.gif) repeat-x; cursor:pointer; height:20px; width:6px;" onclick="window.location.href=\'/to/\'; return false;" onmouseover="fnStatusSet(\'http://www.nxp.com/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;"></td>';sHtml+='<td style="cursor:pointer; height:20px; width:66px;" onclick="window.location.href=\'/to/\'; return false;" onmouseover="fnStatusSet(\'http://www.nxp.com/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;"><img src="http://198.65.99.86/images/nxp/r2c6_r2_applications.gif" alt="" width="66" /></td>';sHtml+='<td style="background:url(http://198.65.99.86/images/nxp/r2c6_r2a.gif) repeat-x; cursor:pointer; height:20px; width:40px;" onclick="window.location.href=\'/to/\'; return false;" onmouseover="fnStatusSet(\'http://www.nxp.com/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;"></td>';sHtml+='<td style="background:url(http://198.65.99.86/images/nxp/r2c6_r2b.gif) repeat-x; cursor:pointer; height:20px; width:6px;" onclick="window.location.href=\'/to/\'; return false;" onmouseover="fnStatusSet(\'http://www.nxp.com/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;"></td>';sHtml+='<td style="cursor:pointer; height:20px; width:60px;" onclick="window.location.href=\'/to/\'; return false;" onmouseover="fnStatusSet(\'http://www.nxp.com/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;"><img src="http://198.65.99.86/images/nxp/r2c6_r2_looking.gif" alt="" width="60" /></td>';sHtml+='<td style="background:url(http://198.65.99.86/images/nxp/r2c6_r2b.gif) repeat-x; cursor:pointer; height:20px; width:40px;" onclick="window.location.href=\'/to/\'; return false;" onmouseover="fnStatusSet(\'http://www.nxp.com/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;"></td>';sHtml+='</tr>';sHtml+='<tr>';sHtml+='<td colspan="3" style="background:url(http://198.65.99.86/images/nxp/r2c6_r3.gif) repeat-x; height:17px;"></td>';sHtml+='<td colspan="3" style="background:url(http://198.65.99.86/images/nxp/r2c6_r3.gif) repeat-x; height:17px;"></td>';sHtml+='</tr>';sHtml+='</table>';document.write(sHtml);}function sFooter(){var sHtml='';sHtml+='<a href="/to/siteowner/" onmouseover="fnStatusSet(\'http://www.nxp.com/siteowner/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">NXP</a> | <a href="/to/privacypolicy/" onmouseover="fnStatusSet(\'http://www.nxp.com/privacypolicy/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">Privacy policy</a> | <a href="/to/terms/" onmouseover="fnStatusSet(\'http://www.nxp.com/terms/\'); return true;" onmouseout="fnStatusSet(\'\'); return true;">Terms of use</a><br />&copy;2006-2008 NXP Semiconductors. All rights reserved. ';document.write(sHtml);}function sXhr(){if(window.XMLHttpRequest){return new XMLHttpRequest();}else if(window.ActiveXObject){var arrVersions=['MSXML2.XMLHttp.5.0','MSXML2.XMLHttp.4.0','MSXML2.XMLHttp.3.0','MSXML2.XMLHttp','Microsoft.XMLHttp'];for(var i=0;i<arrVersions.length;i++){try{return new ActiveXObject(arrVersions[i]);}catch(errXHR){}}}}function sInit(){}function fnStatusSet(strStatusSet){window.status=strStatusSet;}function fnImageChange(strImageName,strImageFile){document.images[strImageName].src=strImageFile;}function sLangToggle(){var sLangSelect=document.getElementById('sLangSelect');if(sLangSelect.style.visibility=='hidden'){if(document.defaultView){var sView=document.defaultView;var sStyleLangDropDown=sView.getComputedStyle(document.getElementById('sLangDropDown'),'');sLangSelect.style.width=sStyleLangDropDown.getPropertyValue('width');}else if(sLangSelect.currentStyle){var sIntWidth=document.body.parentNode.clientWidth;if(sIntWidth>=1000){sIntWidth=150;}else if(sIntWidth<=764){sIntWidth=92;}else{sIntWidth=92+(Math.pow((sIntWidth-764)/236,0.65)*58);}sLangSelect.style.width=sIntWidth+'px';}sLangSelect.style.visibility='visible';}else{sLangSelect.style.visibility='hidden';}}function sLangOff(){var sLangSelect=document.getElementById('sLangSelect');sLangSelect.style.visibility='hidden';}function sTipOff(){var sTip=document.getElementById('sTip');sTip.style.visibility='hidden';}function sTip(sThis,sHtml,sType,sEvent){var sTip=document.getElementById('sTip');if(!sHtml){sTip.style.display='none';sTip.style.visibility='hidden';window.status='';}else{if((sFocus)&&(sXhr)){if(window.event){var sOffsetX=sEvent.clientX+document.documentElement.scrollLeft;var sOffsetY=sEvent.clientY+document.documentElement.scrollTop;}else{var sOffsetX=sEvent.pageX;var sOffsetY=sEvent.pageY;}sTip.style.display='block';if(sType=='img'){{sXhr.open('get',sHtml,true);}{sXhr.onreadystatechange=function(){if(sXhr.readyState==4){if(sXhr.status==200){sTip.innerHTML='<img src="'+sHtml+'" alt="" />';sTip.style.left='-1000px';sTip.style.visibility='visible';sTipMove(sOffsetX,sOffsetY);}}}}{sXhr.send(null);}}else{sTip.innerHTML='sHtml';sTip.style.visibility='visible';}sThis.className='red over';}else{sTip.style.visibility='hidden';}}}var sFocus=1;var sXhr=sXhr();