
//  PARSER.CC  Ver. 1.0
//  Program to extract Nodal equations from Spice Netlist.  Ed Chan
//从 Spice Netlist 中提取节点方程的程序
#include "parser.h"

double stripString( char *stringIn );
void printComponents( Component* compPtr);
void printNodes(Node* nodePtr, int compFlag);
char* strComponentType(Component* compPtr);

 int main( int argc, char *argv[] ) {
  ifstream inFile;
  ofstream outFile;
  NodeHead nodeList;
  CompHead compList;
  ModelHead modelList;

  // Buffers used in parsing:解析中使用的缓冲区
  char inName[NameLength], outName[NameLength], buf[BufLength],
    buf1[BufLength], buf2[BufLength], buf3[BufLength], nameBuf[NameLength],
    *bufPtr, *charPtr1, *charPtr2;
  int intBuf1, intBuf2, intBuf3, intBuf4, datum=NA, eqNum=NA, specPrintJacMNA = 0;
  double douBuf1, douBuf2, douBuf3, douBuf4;
  CompType typeBuf;
  Component *compPtr, *compPtr1, *compPtr2;
  Node *nodePtr, *nodePtr1, *nodePtr2;
  Model* modelPtr;
  TranType TtypeBuf;
  EquaType eqType = Modified;

  strcpy( inName, "Netlist.sp" );
  strcpy( outName, "NO.txt" );

  //  processing command line处理命令行
  if (argc == 1){
    cerr << "USAGE:  parser -f FILENAME <-d DATUM> <-e EQUATION-TYPE> <-o OUTPUT_FILE >" << endl;
    exit(0);
  }
  else {
    for( int c=1; c < argc; c++ ){
      if( (c==1) && isalpha( *argv[c] ) )
            strcpy( inName, argv[c] );
      else if( !strcmp( argv[c], "-f" ) ){
	if( (c+1) > argc ){
	  cerr << "USAGE:  parser -f FILENAME <-d DATUM> <-e EQUATION-TYPE> <-o OUTPUT_FILE>" << endl;
	  exit(0);
	}
	strcpy( inName, argv[c+1] );
      }
      else if( !strcmp( argv[c], "-d" ) ){
	if( (c+1) > argc ){
	  cerr << "USAGE:  parser -f FILENAME <-d DATUM> <-e EQUATION-TYPE> <-o OUTPUT_FILE>" << endl;
	  exit(0);
	}
	datum = atoi( argv[c+1] );
      }
      else if( !strcmp( argv[c], "-e" ) ){
	if( (c+1) > argc ){
	  cerr << "USAGE:  parser -f FILENAME <-d DATUM> <-e EQUATION-TYPE> <-o OUTPUT_FILE>" << endl;
	  exit(0);
	}
	eqNum = atoi( argv[c+1] );
      }
      else if( !strcmp( argv[c], "-o" ) ){
	if( (c+1) > argc ){
	  cerr << "USAGE:  parser -f FILENAME <-d DATUM> <-e EQUATION-TYPE> <-o OUTPUT_FILE>" << endl;
	  exit(0);
	}
	strcpy( outName, argv[c+1] );
      }
    }
  }

  // process equation types:过程方程式类型
  if ( eqNum == NA ){
    while( (eqNum != 1) && (eqNum != 2) ){
      cout << "Available Equations Types Are:" << endl
	   << " <1>  Nodal" << endl
	   << " <2>  Modified Nodal" << endl
	   << "Please enter your choice <1, 2>:" << endl;
      cin >> buf;
      eqNum = atoi( buf );
    }
    if( eqNum == 1 )
      eqType = Nodal;
    else if( eqNum == 2 )
      eqType = Modified;
  }

  // process input file name:进程输入文件名
  if( !strcmp( inName, "Netlist.sp" ) ){
    cerr << "Please enter the input Spice Netlist: <QUIT to exit>" << endl;
    cin >> inName;
    if( !strcmp( inName, "QUIT" ) ){
      cerr << "Program Exited Abnormally!" << endl;
      exit(0);
    }
  }
  inFile.open( inName, ios::in );
  while( !inFile ){
    cerr << inName << " is an invalid input file." << endl
	 << "Please enter the input Spice Netlist: <QUIT to exit>" << endl;
    cin >> inName;
    if( !strcmp( inName, "QUIT" ) ){
      cerr << "Program Exited Abnormally!" << endl;
      exit(0);
    }
    inFile.open( inName, ios::in );
  }

  // process output file:处理输出文件处理输出文件
  if( !strcmp( outName, "NOTHING" ) ){
    strcpy( outName, inName );
    strtok( outName, "." );
    strcat( outName, ".Pout" );
  }
  outFile.open( outName, ios::out );
  cout << "Output saved to file: " << outName << endl;


  // parsing of netlist to create linked list of models (remember to reset the fstream)解析 netlist 以创建模型的链表(记住重置 fstream)
  inFile.getline(buf, BufLength);       // first line of netlist is discarded丢弃网表的第一行
  inFile.getline(buf, BufLength);


  while( inFile.good() ){
    if( (buf == NULL ) || (*buf == '\0') ){
      inFile.getline(buf, BufLength);
      continue;
    }
    strcpy( buf1, buf );
    if( !strcmp( strtok( buf1, " " ), ".model" ) ){
      strcpy( buf2, strtok( NULL, " " ) );
      charPtr1 = strtok( NULL, " " );
      if( !strcmp( charPtr1, "PNP" ) )
            TtypeBuf = PNP;
      else if( !strcmp( charPtr1, "NPN" ) )
            TtypeBuf = NPN;
      else if( !strcmp( charPtr1, "NMOS" ) )
            TtypeBuf = NMOS;
      else if( !strcmp( charPtr1, "PMOS" ) )
	TtypeBuf = PMOS;

      charPtr1 = strtok( NULL, " " );
      while ( charPtr1 != NULL ){
	strcpy( buf3, "" );
	if( (charPtr1[0] == 'I') && (charPtr1[1] == 'S') && (charPtr1[2] == '=') ){
      	  douBuf1 = stripString( charPtr1 );
	}
	if( (charPtr1[0] == 'B') && (charPtr1[1] == 'F') && (charPtr1[2] == '=') ){
	  douBuf2 = stripString( charPtr1 );
	}
	if( (charPtr1[0] == 'B') && (charPtr1[1] == 'R') && (charPtr1[2] == '=') ){
	  douBuf3 = stripString( charPtr1 );
	}
	if( (charPtr1[0] == 'T') && (charPtr1[1] == 'E') && (charPtr1[4] == '=') ){
	  douBuf4 = stripString( charPtr1 );
	}
	charPtr1 = strtok( NULL, " " );
      }
      modelPtr = new Model( buf2, TtypeBuf, douBuf1, douBuf2, douBuf3, douBuf4 );
      modelList.addModel( modelPtr );
    }
    inFile.getline(buf, BufLength);
  }
  inFile.close();
  inFile.open( inName, ios::in );

char model_str[9];
  //  starting of parsing by creating linked list of components通过创建组件的链表开始解析
  inFile.getline(buf, BufLength);       // first line of netlist is discarded丢弃网表的第一行
  inFile.getline(buf, BufLength);
  while( inFile.good() ){
    if( (buf==NULL) || (*buf=='\0') ){
      inFile.getline(buf, BufLength);
      continue;
    }

    if( isalpha( *buf ) ){

      //  EDIT THIS SECTION IF NEW COMPONENTS ARE ADDED!!!
      //  we could do some rearranging in this section to catch each type in order.
      switch( *buf ) {
      case 'v':
      case 'V':
	typeBuf = VSource;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	douBuf1 = atof( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, douBuf1, NA,  intBuf1, intBuf2, NA, NA, NULL, nameBuf );
	compList.addComp( compPtr );
	break;
      case 'i':
      case 'I':
          cout << "I" << endl;
	typeBuf = ISource;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	douBuf1 = atof( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, douBuf1, NA,  intBuf1, intBuf2, NA, NA, NULL, nameBuf );
	compList.addComp( compPtr );
	break;
      case 'q':
      case 'Q':
	typeBuf = BJT;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	intBuf3 = atoi( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, NA, NA, intBuf1, intBuf2, intBuf3, NA,
				 modelList.getModel( strtok( NULL, " " ) ), nameBuf );
	compList.addComp( compPtr );
	break;
      case 'm':
      case 'M':
	typeBuf = MOSFET;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	intBuf3 = atoi( strtok( NULL, " " ) );
	intBuf4 = atoi( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, NA, NA, intBuf1, intBuf2, intBuf3, intBuf4,
				 modelList.getModel( strtok( NULL, " " ) ), nameBuf );
	compList.addComp( compPtr );
	break;
      case 'r':
      case 'R':
	typeBuf = Resistor;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	douBuf1 = atof( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf );
	compList.addComp( compPtr );
	break;
      case 'd':
      case 'D':
	typeBuf = Diode;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	charPtr1 = strtok( NULL, " " );
	while( charPtr1 != NULL ){
	  if( (charPtr1[0] == 'I') && (charPtr1[1] == 'S') && (charPtr1[2] == '=') ){
	    douBuf1 = stripString( charPtr1 );
	  }
	  if( (charPtr1[0] == 'T') && (charPtr1[1] == 'E') && (charPtr1[4] == '=') ){
	    douBuf2 = stripString( charPtr1 );
	  }
	  charPtr1 = strtok( NULL, " " );
	}
	compPtr = new Component( typeBuf, douBuf1, douBuf2, intBuf1, intBuf2, NA, NA, NULL, nameBuf );
	compList.addComp( compPtr );
	break;
      case 'c':
      case 'C':
	typeBuf = Capacitor;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	douBuf1 = atof( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
	compList.addComp( compPtr );
	break;
      case 'l':
      case 'L':
	typeBuf = Inductor;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	douBuf1 = atof( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
	compList.addComp( compPtr );
	break;
      };
    }
    inFile.getline(buf, BufLength);
  }


//  Now the components are created and it is time to set up the list of nodes.
//  we should actually use second connector of first Source as the first Node (Datum)
  compPtr1 = compList.getComp(0);
  while( compPtr1 != NULL ){
	for ( int b=0; b < 3; b++ ){ /* ~> J. Erik Melo note: A component can have until 4 connectors. But here just 3 are been considered. It should change the condition to 'b <= 3' or 'b < 4'?*/
	  if( (!compPtr1->isCon( b )) && ( compPtr1->getConVal( b ) != NA ) ){ //~> verify if the connector 'b' is not set && if the name of the node to which this same connector 'b' is connected is a valid name as found in the circuit file. That is, if the name is not NA, that is, if this connector was named in the instantiation of the component.
	intBuf1 = compPtr1->getConVal( b ); // ~> getting the connector number as in the netlist file
	nodePtr1 = nodeList.addNode();
	nodePtr1->setNameNum( intBuf1 );  // ~> naming the node as in the netlist file
	compPtr1->connect( b, nodePtr1 ); // ~> connecting the 'connector' of component to the node
	nodePtr1->connect( b, compPtr1 ); // ~> connecting the 'connection' of the node to the component

	// now search and connect all other appropriate connectors to this node.
	// error checking should be added to prevent duplicated, or skipped connectors.
	compPtr2 = compPtr1->getNext();
	while( compPtr2 != NULL ){
	  for( int c=0; c < 3; c++ ){ //~> verifying which one of the others connectors (of components) are connected to the node above
	    if( compPtr2->getConVal( c ) == intBuf1  ){ //~> if next component in the list of components has a connector with the same name (conNum) of the connector above, connect it to the same node.
	      compPtr2->connect( c, nodePtr1 );
	      nodePtr1->connect( c, compPtr2 );
	      break;                                    //~> As a component can only have one connector with the same name (connected in the same node), don't search the others and go out of the 'for' loop
	    }
	  }
	  compPtr2 = compPtr2->getNext();
	}
      }
    }
    compPtr1 = compPtr1->getNext();
  }

//  At this point, we are done creating a representation of the circuit in memory
//  now, we need to call each node to create and output its nodal equation.
//  Each node will call the components attached for the individual contributions to the
//  nodal equation.

  // verify that input datum is valid
  Boolean check = FALSE;
  if( datum != NA ){
    nodePtr = nodeList.getNode(0);
    while( nodePtr != NULL ){
      if( nodePtr->getNameNum() == datum )
	check = TRUE;
      nodePtr = nodePtr->getNext();
    }
    if( check == FALSE ){
      cerr << "Datum value invalid!" << endl
	   << "PROGRAM EXITED ABNORMALLY!" << endl;
      exit(0);
    }
  }

// Loop to find lastnode
  nodePtr = nodeList.getNode(0); //~> getting the pointer to the first node, pointed by 'headNode'
  int lastnode = nodePtr->getNameNum();
  while( nodePtr != NULL ){
    lastnode = ( nodePtr->getNameNum() > lastnode )? nodePtr->getNameNum(): lastnode;
    nodePtr = nodePtr->getNext();
  }

//  Loop to find the datum
  if( datum == NA ){
    nodePtr = nodeList.getNode(0);
    nodePtr1 = nodePtr->getNext();
    while( nodePtr1 != NULL ){
      if( nodePtr1->getCount() > nodePtr->getCount() )
	nodePtr = nodePtr1;
      nodePtr1 = nodePtr1->getNext();
    }
    datum = nodePtr->getNameNum();
  }
//=================================
//~> Checking the component list
//~> Comment this part to omit
    compPtr = compList.getComp(0);
    printComponents(compPtr);

    nodePtr = nodeList.getNode(0);
    printNodes(nodePtr, 1);

//<~
//==================================

  // output circuit information
  outFile << "%Parser V1.0" << endl;
  outFile << "%Input Spice Deck:  " << inName << endl;
  outFile << "%Equation Type:     ";
  if( eqType == Nodal )
    outFile << "NODAL" << endl;
  else if( eqType == Modified )
    outFile << "MODIFIED NODAL" << endl;
  outFile << "%Datum Node:        " << datum << endl;


  // create value table
  outFile << endl
	  << "%*****************************************************************************" << endl;
  outFile << "%                      Component Values:" << endl;
  compPtr = compList.getComp(0);
  while ( compPtr != NULL ){
    compPtr->printVal( outFile );
    compPtr = compPtr->getNext();
  }
  outFile << endl
	  << "%*****************************************************************************" << endl;


  // go down the nodal list and have components announce themselves
  outFile << endl << "%                      Circuit Equations: " << endl;
  nodePtr = nodeList.getNode(0);
  while( nodePtr != NULL ){
    if( nodePtr->getNameNum() != datum ){
      nodePtr->printNodal( outFile, datum, lastnode );
    }
    nodePtr = nodePtr->getNext();
  }

  //go down the component list and give equations for all sources
  compPtr = compList.getComp(0);
  while( compPtr != NULL ) {
    compPtr->specialPrint ( outFile, datum );
    compPtr = compPtr->getNext();
  }

   //~> go down the component list and give supernode equations for all float sources (Nodal Analysis)
   if(eqType != Modified){
      compPtr = compList.getComp(0);
      while( compPtr != NULL ) {
         compPtr->printSuperNode ( outFile, datum, lastnode );
         compPtr = compPtr->getNext();
        }
   }

  // go down the node list and give additional MNA equations
  if (eqType == Modified) {
    nodePtr = nodeList.getNode(0);
    while( nodePtr != NULL ){
      if (nodePtr->getNameNum() != datum)
        nodePtr->printMNA( outFile, datum, lastnode );
      nodePtr = nodePtr->getNext();
    }
  }

  // print jacobians
  outFile << endl
	<< "%*****************************************************************************" << endl;
  outFile << endl << "%                      Jacobians: " << endl;
  nodePtr1 = nodeList.getNode(0);
  while ( nodePtr1 != NULL) {   //~> this loop handles the nodes not connected to a Vsource and those ones that are not the 'datum' node
    if (nodePtr1->getNameNum() != datum) {
      nodePtr2 = nodeList.getNode(0);
      while ( nodePtr2 != NULL) {
        if ( nodePtr2->getNameNum() != datum) {
          nodePtr1->printJac(outFile, datum, nodePtr2, lastnode, eqType);
        }
        nodePtr2 = nodePtr2->getNext();
      }
    }
    nodePtr1 = nodePtr1->getNext();
  }

  // go down the component list and give equations for all sources
  compPtr = compList.getComp(0);
  while( compPtr != NULL ){
    nodePtr2 = nodeList.getNode(0);
    compPtr2 = compList.getComp(0);
    while( nodePtr2 != NULL ){
      if( nodePtr2->getNameNum() != datum ){
        compPtr->specialPrintJac( outFile, datum, nodePtr2/**/, lastnode, eqType, compPtr2, &specPrintJacMNA /**/ ); // ~> specPrintJacMNA is used to verify if the jacobians w.r.t. the Modified equations was already printed to print only once.
      }
      nodePtr2 = nodePtr2->getNext();
    }
    specPrintJacMNA = 0;
    compPtr = compPtr->getNext();
  }




  // print the Jacobians for the additional MNA equations
  if (eqType == Modified) {
    nodePtr1 = nodeList.getNode(0);
    while( nodePtr1 != NULL) {
      if ( nodePtr1->getNameNum() != datum) {
        nodePtr2 = nodeList.getNode(0);
        while (nodePtr2 != NULL) {
          if ( nodePtr2->getNameNum() != datum)
            nodePtr1->printJacMNA( outFile, datum, nodePtr2, lastnode );
          nodePtr2 = nodePtr2->getNext();
        }
      }
      nodePtr1 = nodePtr1->getNext();
    }
  }


  cout << endl << "That's all folks!" << endl;
  return 0;
}


double stripString( char *stringIn ){
  char buf[BufLength], buf2[BufLength];
  int a, b;
  strcpy( buf, stringIn );
  for( a=0; buf[a] != '='; a++ ){};
  a++;
  for( b=0; buf[a] != '\0'; b++, a++ )
    buf2[b] = buf[a];
  buf2[b] = '\0';
  return atof( buf2 );
};


//Print the linked list of components to check
void printComponents( Component* compPtr){
    char compTypeName[6];
    cout << endl << "Components: " << endl << endl;
    while(compPtr != NULL){
        strcpy(compTypeName, strComponentType(compPtr));
        cout << "->" << compTypeName << compPtr->getcompNum();
        compPtr = compPtr->getNext();
    }
    cout << endl;
    return;
}

void printNodes(Node* nodePtr, int compFlag){

    Connections* conPtr;
    cout << endl << "Nodes: " << endl << endl;
    while(nodePtr != NULL){
        if(compFlag == 0){ //It is printed just the names of the nodes
            cout << "-> " << nodePtr->getNameNum();
        }
        else if (compFlag == 1){ //It is printed the nodes and the connections
            cout << "-> " << nodePtr->getNameNum() << " {";
                conPtr = nodePtr->getConList();
            while(conPtr->next != NULL){
                cout << strComponentType(conPtr->comp) << conPtr->comp->getcompNum() << ", ";
                conPtr = conPtr->next;
            }
            cout << strComponentType(conPtr->comp) << conPtr->comp->getcompNum() << '}' << endl;
        }
        else{
            cout << "Invalid value for compFlag. (0) to print just nodes, (1) to print nodes and connections!";
            exit(1);

        }

        nodePtr = nodePtr->getNext();
    }


    return;
}


char* strComponentType(Component* compPtr){

    char* compTypeName = new char[6];
    switch(compPtr->getType()){

        case VSource: strcpy(compTypeName, "V"); break;
        case Resistor: strcpy(compTypeName, "R"); break;
        case BJT: strcpy(compTypeName, "T"); break;
        case MOSFET: strcpy(compTypeName, "M"); break;
        case ISource: strcpy(compTypeName, "I"); break;
        case Inductor: strcpy(compTypeName, "ind"); break;
        case Diode: strcpy(compTypeName, "Diode"); break;
        case Capacitor: strcpy(compTypeName, "Cap"); break;
    }

    return compTypeName;
}





