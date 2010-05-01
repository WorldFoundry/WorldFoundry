#===============================================================================
# GNUmakefile for attribedit: Copyright (c) 1999, World Foundry Group  
# Part of the World Foundry 3D video game engine/production environment
# for more information about World Foundry, see www.worldfoundry.org
#==============================================================================
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# Version 2 as published by the Free Software Foundation
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# or see www.fsf.org
#==============================================================================
# if you uncomment the 2 lines with 'xml' in them below then you will need to 
# get libxmlio (found at http://www.fxtech.com/xmlio/index.html)
#==============================================================================

PROGRAM = attribedit

OBJ_DIR = objs/

ALL :  $(OBJ_DIR) $(PROGRAM)

OBJS = \
	$(OBJ_DIR)$(PROGRAM)$(OBJ) \
	$(OBJ_DIR)gui$(OBJ) \
	$(OBJ_DIR)guihelpers$(OBJ) \
	$(OBJ_DIR)aegui$(OBJ) \
        $(OBJ_DIR)oadesciffbinparser$(OBJ) \
        $(OBJ_DIR)gladexmlgenerator$(OBJ) \
        $(OBJ_DIR)oad$(OBJ) \
        $(OBJ_DIR)iffhelpers$(OBJ) \


#==============================================================================
# kts I know, I need to learn how to use autoconf so that this is set correctly automatically

# this set is for Mandrake
#SIGCPP_BASE = /usr/
#GTKMM_DIR = /usr/lib/gtkmm/include

#this set is for Red Hat and others
SIGCPP_BASE = /usr/local/
GTKMM_DIR = /usr/local/lib/gtkmm/include

# don't remember which OS this is for
#GTKMM_DIR = /usr/include/gtk

GTK_INCLUDE_DIR = /usr/include/gtk-1.2
GLIB_INCLUDE_DIR = /usr/include/glib-1.2
GLIB_LIB_INCLUDE_DIR = /usr/lib/glib/include

SIGCPP_INCLUDE = $(SIGCPP_BASE)include/sigc++-1.0
SIGCPP_LIB_INCLUDE = $(SIGCPP_BASE)/lib/sigc++-1.0/include

#CCCARGS += -DUSEXML
CCCARGS += -I $(SIGCPP_INCLUDE) -I $(SIGCPP_LIB_INCLUDE) 
CCCARGS += -I $(GTK_INCLUDE_DIR) -I $(GTKMM_DIR) -I . -I /usr/include 
CCCARGS += -I $(GLIB_INCLUDE_DIR) -I $(GLIB_LIB_INCLUDE_DIR)
CCCARGS += -I $(GTKMM_INCLUDE_DIR) 
CCCARGS += $(CXX_FLAGS) $(SIGC_CFLAGS) $(GTK_CFLAGS)

LNARGS += -lgtkmm -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext \
	 -lgdkmm -lgtk -lgdk \
	 -rdynamic -lgmodule -lglib -ldl -lXi -lXext -lsigc -lpthread 
#LNARGS += -lxmlio


##############################################################################

include ../GNUMakefile.tool

##############################################################################

clean : tool-clean

##############################################################################
