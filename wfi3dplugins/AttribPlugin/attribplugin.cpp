/***************************************************************************
                          AttribPlugin.cpp  -  description
        By Kevin T. Seghetti (C) Copyright 2001 Released under GPL

     Based on plugin example code:                          
                             -------------------
    begin                : Sun Aug 20 2000
    copyright            : (C) 2000 by Jon Anderson
    email                : janderson@onelink.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "attribplugin.h"
#include "profile.hp"
#include "wfattribdata.hp"
#include <qfiledialog.h>
#include <qvalidator.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>

//==============================================================================

extern "C" {
  DialogPlugin * i3d_createInstance()
  {
	return new AttribPlugin();
  }
}

AttribPlugin::AttribPlugin() : DialogPlugin( "Attrib" ),  AttribEditDialog()
{
//  fldWidth -> setValidator(new QIntValidator(0, 2000, fldWidth));
//  fldHeight -> setValidator(new QIntValidator(0, 2000, fldHeight));
  cout << "AttribPlugin constructed!" << endl;
}

//==============================================================================

AttribPlugin::~AttribPlugin()
{
    // no need to delete child widgets, Qt does it all for us
}

//==============================================================================

void AttribPlugin::DoEdit()
{
	vector<Selectable *>* list;
	list = ObjectDB::getInstance()->getSelected();

   // kts for now just edit the last one selected
   Entity* entity = NULL;

	for(int i=0; i<(int)list->size(); i++)
   {	
      if((*list)[i]->isA(Object::TYPE) ) 
      {
         entity = (Entity *) (*list)[i];
      }	
   }

   if(entity)
   {
      // create temporary file name
      string tempFileName = tmpnam(NULL);
      tempFileName += "WorldFoundry";        // ought to be unique now

      //string editorPath = "/usr/local/src/WorldFoundry/wfsource/bin/attribedit";

      char charBuffer[1024];
      string wfConfigPath = getenv("HOME");
      wfConfigPath += "/.worldfoundry/";
      wfConfigPath += "/attribedit.ini";

      int retVal = GetPrivateProfileString("dirs", "bindir", "default", charBuffer, 1024, wfConfigPath.c_str() ); 
      if(retVal == -1)
      {
         cout << "Unable to load bindir from section dirs in ini file < " << wfConfigPath << ">" << endl; 
         return;
      }  

      string editorPath(charBuffer);
      editorPath += "/attribedit";

      string command;
      command += editorPath;
      command += " --outputname=";
      command += tempFileName;
      command += " --objectname=\"";
      command += entity->getName();
      command += "\"";

       // fetch attribute data if there is any
      IData* origData = (entity->getData(ATTRIBUTEDATANAME));
      WorldFoundryAttributeData* data = dynamic_cast<WorldFoundryAttributeData*>(origData);
      //cout << "origData = " << origData << ", data = " << data << endl;

      remove(tempFileName.c_str());          // just in case
      if(data)
      {
         // write it to a file if found
         ofstream out(tempFileName.c_str());
         assert(out.good());
         const string& dStr = data->toString();
         cout << "data found:\n" << dStr << endl;
         out.write(dStr.c_str(),dStr.size());

         command += " ";
         command += tempFileName;
      }
      else
         cout << "no data found, creating" << endl;

      cout << "executing " << command << endl;
      int result = system(command.c_str());
      if(result == 0)
      {
          // read it back
         {
            ifstream in(tempFileName.c_str());
            if(in.good())                          // if file not there user must have canceled
            {
               string resultString;
               getline(in, resultString,'\0');
               cout << "result = " << resultString << endl;

               if(data)
                  data->Set(resultString);
               else
               {

                  WorldFoundryAttributeData* result = new WorldFoundryAttributeData(resultString);
                  entity->putData(  ATTRIBUTEDATANAME , result );
                  //IData* foo = entity->getData(adn);
                  //cout << "put with " << adn << ", read back " << foo << endl;
                  // test code

//                  IDataMap m_data;
//                   cout << "size = " << m_data.size() <<endl;
//                   m_data.insert( IDataEntry( adn , result ) );
//                   cout << "size = " << m_data.size() <<endl;
//                   IData* foo2 = m_data[adn];
//                   cout << "foo = " << foo2 << endl;
               }
            }
         }


      }
      else
         cerr << "error executing attribedit: " << result << endl;

      remove(tempFileName.c_str());         // all done with this temp file

      //assert(result == 0);
   }
   else
      cout << "no objects selected!" << endl;
}


//==============================================================================


void AttribPlugin:: startPlugin()
{
#if 1
   DoEdit();
#else 
	show();
#endif
}
void AttribPlugin::stopPlugin()
{

	hide();
}

//==============================================================================

