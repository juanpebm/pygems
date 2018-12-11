/*
 * Copyright (c) 2017 CCS/GMRV/URJC/UPM.
 *
 * Authors: Juan P. Brito <juanpedro.brito@upm.es>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <iostream>
#include <vector>
#include <assert.h>

#include <nsol/nsol.h>

#include <PyGEmSManager/StrategyFrameworkBPModule.h>
#include <PyGEmSManager/PyGEmSManager.h>

using namespace NSPyGEmS;
using namespace NSPGManager;

const unsigned int vecSize = 10;
unsigned int numResizedNodesInPython = 15;

using namespace nsol;

void traverseTracing( NeuronMorphologyPtr morpho )
{
  for( auto neurite: morpho->neurites( ))
  {
    std::cout << "=== Neurite ===" << std::endl;
    for( auto section: neurite->sections( ))
    {
      std::cout << "==== Section ===" << std::endl;
      int firstSectionId = section->backwardNode( )->id( );
      for( auto node: section->nodes( ))
      {
        int parentId;
        if( firstSectionId == node->id( ))
        {
          parentId = -1; //problem
        }
        else
        {
          parentId = node->id( ) - 1;
        }
        std::cout << "Node: " << node->id( );
        std::cout << " Parent: " << parentId;
        std::cout << " Coords: (" << node->point( ).x( ) << " , "
                  << node->point( ).y( ) << " , "
                  << node->point( ).z( )
                  << " , " << ")";
        std::cout << " Radius:" << node->radius( );
        std::cout << std::endl;
      }
    }
  }
}

int main( int argc, char* argv[] )
{
  if( argc != 3 )
  {
    std::cout << "This test requieres 2 parameters: "
                 "strategy_file num_resized_nodes_in_python." << std::endl;
    return 1;
  }
  else
  {
    numResizedNodesInPython = atoi( argv[2] );
  }
  try
  {
    //BPCode
#ifdef PYGEMS_USE_PYTHON3
    //Python >=3
    PyGEmSManager myPyGEmSManager( "StrFramework", &PyInit_StrFramework,
                                   "Strategies", std::string( argv[1] ));
#else
    //Python 2.7
    PyGEmSManager myPyGEmSManager( "StrFramework", &initStrFramework,
                                   "Strategies",  std::string( argv[1] ));
#endif
    bp::object Strategy = myPyGEmSManager.getModuleAttrib( "Strategy" );

    Container _Container;
    StrategyParams lStrategyParams;
    for( unsigned int i = 0; i < vecSize; ++i )
    {
      lStrategyParams.stringParam =
        "MyTest_" + std::to_string( lStrategyParams.intParam ) + "_";
      lStrategyParams.intParam++;
      _Container.addElement( lStrategyParams );
    }
    bp::object _strategy = Strategy( _Container );

    std::cout
      << "Initial vector (Showing only string (from base class) parameter)."
      << std::endl;
    for( unsigned int i = 0; i < vecSize; ++i )
    {
      std::cout << _Container.getContainer( )[i].stringParam << std::endl;
    }
    std::string injectedVarName = "vecIn";
    std::string injectedVarName3 = "listOut";

    //BPCode
    myPyGEmSManager.getModule( ).attr( injectedVarName.c_str( )) =
      _Container.getContainer( );
    myPyGEmSManager.getModule( ).attr( injectedVarName3.c_str( )) = 0;

    std::cout << "Initial vector dimensions. Value:"
              << _Container.getContainer( ).size( ) << std::endl;

    std::cout << "Set new dimensions of the vector to python. Value:"
              << numResizedNodesInPython << std::endl;
    _strategy.attr( "setContainerResizeDimension" )( numResizedNodesInPython );

    std::cout << "Calling python methods from CPP." << std::endl;

    _strategy.attr( "simplify" )( );
    _strategy.attr( "fix" )( );
    _strategy.attr( "enhance" )( );
    _strategy.attr( "userDefined" )( );

    std::cout << "Recovering new container from Python." << std::endl;
    bp::list result =
      myPyGEmSManager.extractListFromModule( injectedVarName3.c_str( ));

    int n = bp::len( result );
    std::cout << "Container recovered dimensions Value:" << n << std::endl;

    std::cout << "Showing received string value (modified on python):"
              << std::endl;
    for( int i = 0; i < n; ++i )
    {
      StrategyParams val = bp::extract < StrategyParams >( result[i] );
      std::cout << val.stringParam << std::endl;;
    }

    //Ping pong test
    std::cout << "Starting ping-pong test:" << std::endl;
    _strategy.attr( "eval" )( );

    std::cout << std::endl;
    return 0;
  }
  catch( const bp::error_already_set& )
  {
    std::cerr << ">>> Error! Uncaught exception:\n";
    PyErr_Print( );
    return 1;
  }
}
