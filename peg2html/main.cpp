//
//  main.cpp
//  peg2html
//
//  Created by Alexandre Cossette on 11-11-03.
//  Copyright (c) 2011 alexk7. All rights reserved.
//

#include <iostream>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <boost/format.hpp>
#include "PEGParser.h"

using namespace std;
using namespace boost;
using namespace PEGParser;

static string MangleId(string _id)
{
    string result;
    for (size_t i = 0, iEnd = _id.size(); i != iEnd; ++i)
    {
        char c = _id[i];
        if (isupper(c))
        {
            result.push_back('_');
            result.push_back(tolower(c));
        }
        else
        {
            result.push_back(c);
        }
    }
    return result;
}

static void WriteIdUseHtml(string _id)
{
    cout << format("<a class='%1%' href='#%1%' onclick='Highlight(\"%1%\")'>%2%</a>") % MangleId(_id) % _id;
}

static void WriteArrowHtml(char _arrowType)
{
    cout << " <" << _arrowType << " ";
}

static void WriteExprHtml(Iterator _iExpr)
{
	assert(_iExpr->type == SymbolType_Expression);
	
    bool choiceFirst = true;
	for (Iterator iSeq = Traverse(_iExpr); iSeq->type == SymbolType_Sequence; ++iSeq)
	{
        if (choiceFirst)
            choiceFirst = false;
        else
            cout << " / ";

        bool seqFirst = true;
		for (Iterator iItem = Traverse(iSeq); iItem->type == SymbolType_Item; ++iItem)
		{
            if (seqFirst)
                seqFirst = false;
            else
                cout << " ";
            
			char cPrefix = *iItem->value;
			if (cPrefix == '&' || cPrefix == '!')
			{
                cout << cPrefix;
			}
			
			Iterator iPrimary = Traverse(iItem);
			assert(iPrimary->type == SymbolType_Primary);
			Iterator i = Traverse(iPrimary);
			if (i->type == SymbolType_Identifier)
			{
				WriteIdUseHtml(std::string(i->value, i->length));
			}
			else if (i->type == SymbolType_Expression)
			{
                cout << "(";
                WriteExprHtml(i);
                cout << ")";
			}
			else if (i->type == SymbolType_LITERAL)
			{
                cout << '"';
				for (Iterator iChar = Traverse(i); iChar->type == SymbolType_Char; ++iChar)
				{
					cout.write(iChar->value, iChar->length);
				}
                cout << "\"";
			}
			else if (i->type == SymbolType_CLASS)
			{
                cout << '[';
				for (Iterator iRange = Traverse(i); iRange->type == SymbolType_Range; ++iRange)
				{
					Iterator iChar = Traverse(iRange);
					assert(iChar->type == SymbolType_Char);
					cout.write(iChar->value, iChar->length);
					if ((++iChar)->type == SymbolType_Char)
					{
                        cout << '-';
						cout.write(iChar->value, iChar->length);
					}
				}
                cout << "]";
			}
			else
			{
				cout << ".";
			}
			
			char cSuffix = iPrimary->value[iPrimary->length];
			if (cSuffix == '?' || cSuffix == '*' || cSuffix == '+')
			{
                cout << cSuffix;
			}
		}
	}
}

int main (int argc, const char * argv[])
{
    ifstream in;
    if (argc > 1)
    {
        in.open(argv[1]);
        cin.rdbuf(in.rdbuf());
    }
    
    ofstream out;
    if (argc > 2)
    {
        out.open(argv[2]);
        cout.rdbuf(out.rdbuf());
    }
    
    vector<char> text;
    text.reserve(4096);
    istreambuf_iterator<char> itrIn(cin), itrInEnd;
    copy(itrIn, itrInEnd, back_inserter(text));
    text.push_back(0);
    
    Iterator iGrammar = Traverse(SymbolType_Grammar, &text[0]);
    Iterator iDef = Traverse(iGrammar);
    if (iDef->type != SymbolType_Definition)
        return 0;
    
    Iterator i = Traverse(iDef);
    assert(i->type == SymbolType_Identifier);
    
    cout << "<style>\n";
    cout << "body {font:normal normal 100%/1.2 courier;}\n";
    cout << "a {text-decoration:none; color:black;}\n";
    cout << "a:hover {font-weight:bold;}\n";
    cout << format(".%1% {font-weight:bold; background-color:yellow;}\n") % MangleId(std::string(i->value, i->length));
    cout << "</style>\n";
    cout << "<script>\n";
    cout << "function Highlight(id) {\n";
    cout << "    var sheet = document.styleSheets[0];\n";
    cout << "    var index = sheet.cssRules.length-1;\n";
    cout << "    sheet.deleteRule(index);\n";
    cout << "    sheet.insertRule(\".\" + id + \" {font-weight:bold; background-color:yellow;}\", index);\n";
    cout << "}\n";
    cout << "</script>\n";
    cout << "<ol>\n";
    
	for (; iDef->type == SymbolType_Definition; ++iDef)
	{
		i = Traverse(iDef);
		assert(i->type == SymbolType_Identifier);
		std::string id(i->value, i->length);
		char arrowType = (++i)->value[1];
		
        cout << format("<li><a class='%1%' href='#%1%' id='%1%' onclick='Highlight(\"%1%\")'>%2%</a>") % MangleId(id) % id;
        WriteArrowHtml(arrowType);
		WriteExprHtml(++i);
        cout << "</li>\n";
	}
    
    cout << "</ol>\n";
    return 0;
}

