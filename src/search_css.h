"/*---------------- Search Box */\n"
"\n"
"#MSearchBox {	\n"
"    padding: 0px;\n"
"    margin: 0px;\n"
"    border: none;\n"
"    border: 1px solid #84B0C7;\n"
"    white-space: nowrap;\n"
"    -moz-border-radius: 8px;\n"
"    -webkit-border-top-left-radius: 8px;\n"
"    -webkit-border-top-right-radius: 8px;\n"
"    -webkit-border-bottom-left-radius: 8px;\n"
"    -webkit-border-bottom-right-radius: 8px;\n"
"}\n"
"#MSearchField {\n"
"    font: 9pt Arial, Verdana, sans-serif;\n"
"    color: #999999;\n"
"    background-color: #FFFFFF;\n"
"    font-style: normal;\n"
"    cursor: pointer;\n"
"    padding: 0px 1px;\n"
"    margin: 0px 6px 0px 0px;\n"
"    border: none;\n"
"    outline: none;\n"
"}\n"
"#MSearchBox.MSearchBoxInactive:hover #MSearchField {\n"
"    background-color: #FFFFFF;\n"
"    padding: 0px 1px;\n"
"    margin: 0px 6px 0px 0px;\n"
"    border: none;\n"
"}\n"
".MSearchBoxActive #MSearchField {\n"
"    background-color: #FFFFFF;\n"
"    color: #000000;\n"
"    padding: 0px 1px;\n"
"    margin: 0px 6px 0px 0px;\n"
"    border: none;\n"
"}\n"
"#MSearchSelect {\n"
"    float : none;\n"
"    display : inline;\n"
"    background : none;\n"
"    font: 9pt Verdana, sans-serif;\n"
"    margin-right: -6px;\n"
"    padding: 0;\n"
"    border: none;\n"
"    margin: 0px 0px 0px 6px;\n"
"    vertical-align: bottom;\n"
"    padding: 0px 0px;\n"
"    background-color: #84B0C7;\n"
"}\n"
".MSearchBoxLeft {\n"
"    display: block;\n"
"    text-align: left;\n"
"    float: left;\n"
"    margin-left: 6px;\n"
"}\n"
".MSearchBoxRight {\n"
"    display: block;\n"
"    float: right;\n"
"    text-align: right;\n"
"    margin-right: 6px;\n"
"}\n"
".MSearchBoxSpacer {\n"
"    font-size: 0px;\n"
"    clear: both;\n"
"}\n"
".MSearchBoxRow {\n"
"    font-size: 0px;\n"
"    clear: both;\n"
"}\n"
"\n"
"/*---------------- Search filter selection */\n"
"\n"
"#MSearchSelectWindow {\n"
"    display: none;\n"
"    position: absolute;\n"
"    left: 0; top: 0;\n"
"    border: 1px solid #A0A0A0;\n"
"    background-color: #FAFAFA;\n"
"    z-index: 1;\n"
"    padding-top: 4px;\n"
"    padding-bottom: 4px;\n"
"    -moz-border-radius: 4px;\n"
"    -webkit-border-top-left-radius: 4px;\n"
"    -webkit-border-top-right-radius: 4px;\n"
"    -webkit-border-bottom-left-radius: 4px;\n"
"    -webkit-border-bottom-right-radius: 4px;\n"
"    -webkit-box-shadow: 5px 5px 5px rgba(0, 0, 0, 0.15);\n"
"    }\n"
".SelectItem {\n"
"    font: 8pt Arial, Verdana, sans-serif;\n"
"    padding-left:  2px;\n"
"    padding-right: 12px;\n"
"    border: 0px;\n"
"}\n"
"span.SelectionMark {\n"
"    margin-right: 4px;\n"
"    font-family: monospace;\n"
"    outline-style: none;\n"
"    text-decoration: none;\n"
"}\n"
"a.SelectItem {\n"
"    display: block;\n"
"    outline-style: none;\n"
"    color: #000000; \n"
"    text-decoration: none;\n"
"    padding-left:   6px;\n"
"    padding-right: 12px;\n"
"}\n"
"a.SelectItem:visited,\n"
"a.SelectItem:active {\n"
"    color: #000000; \n"
"    outline-style: none;\n"
"    text-decoration: none;\n"
"}\n"
"a.SelectItem:hover {\n"
"    color: #FFFFFF;\n"
"    background-color: #2A50E4;\n"
"    outline-style: none;\n"
"    text-decoration: none;\n"
"    cursor: pointer;\n"
"    display: block;\n"
"}\n"
"\n"
"/*---------------- Search results window */\n"
"\n"
"iframe#MSearchResults {\n"
"    width: 60ex;\n"
"    height: 15em;\n"
"    }\n"
"#MSearchResultsWindow {\n"
"    display: none;\n"
"    position: absolute;\n"
"    left: 0; top: 0;\n"
"    border: 1px solid #000000;\n"
"    background-color: #EEF3F5;\n"
"    }\n"
"#MSearchResultsWindowClose {\n"
"    font-weight: bold;\n"
"    font-size: 8pt;\n"
"    display: block;\n"
"    padding: 0px;\n"
"    margin: 0px;\n"
"    text-align: right;\n"
"    text-decoration: none;\n"
"    outline-style: none;\n"
"    }\n"
"#MSearchResultsWindowClose:link,\n"
"#MSearchResultsWindowClose:visited {\n"
"    color: #8A8A8A;\n"
"    background-color: #8A8A8A;\n"
"    }\n"
"#MSearchResultsWindowClose:active,\n"
"#MSearchResultsWindowClose:hover {\n"
"    color: #9E9E9E;\n"
"    background-color: #9E9E9E;\n"
"    }\n"
"\n"
"/* ----------------------------------- */\n"
"\n"
"\n"
"#SRIndex {\n"
"    clear:both; \n"
"    padding-bottom: 15px;\n"
"}\n"
"\n"
".SREntry {\n"
"    font-size: 10pt;\n"
"    padding-left: 1ex;\n"
"}\n"
".SRPage .SREntry {\n"
"    font-size: 8pt;\n"
"    padding: 1px 5px;\n"
"}\n"
"body.SRPage {\n"
"    margin: 5px 2px;\n"
"}\n"
"\n"
".SRChildren {\n"
"    padding-left: 3ex; padding-bottom: .5em \n"
"}\n"
".SRPage .SRChildren {\n"
"    display: none;\n"
"}\n"
".SRSymbol {\n"
"    font-weight: bold; color: #153788;\n"
"    font-family: Arial, Verdana, sans-serif;\n"
"    text-decoration: none;\n"
"    outline: none;\n"
"}\n"
"\n"
"a.SRScope {\n"
"    display: block;\n"
"    color: #153788; \n"
"    font-family: Arial, Verdana, sans-serif;\n"
"    text-decoration: none;\n"
"    outline: none;\n"
"}\n"
"\n"
".SRPage .SRStatus {\n"
"    padding: 2px 5px;\n"
"    font-size: 8pt;\n"
"    font-style: italic;\n"
"}\n"
"\n"
".SRResult {\n"
"    display: none;\n"
"}\n"
"\n"
