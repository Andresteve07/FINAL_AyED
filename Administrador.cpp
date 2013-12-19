#include "Administrador.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <string.h>
#include <set>

#define INF 9999
#define TAM_MAX_PAG 100
#define TAM_MIN_PAG 10

Administrador::Administrador()
	:routers(),computadoras(),arcos(),conexiones(),cant_comp_por_router(0),cant_routers(0),total_pag(0){}

Administrador::~Administrador(){
}

void Administrador::simular_cant_pasos(int cant_pasos){
		for(int i=0; i<cant_pasos; i++){
			this->simular_un_paso();
		}
}

void Administrador::crear_pagina(){
//Cada cierta cantidad de pasos de simulacion (5) se generaran paginas aleatorioas de compuatadoras aleatorias.
//
	srand(time(0));
	int i = rand() % (cant_comp_por_router+1);
	int j = rand() % (cant_routers+1);
	int arreglo1[2]={i,j};
	vector<int> ip_comp_origen(arreglo1,arreglo1+sizeof(arreglo1)/sizeof(arreglo1[0]));
	srand(time(0));
	int k = rand() % (cant_comp_por_router+1);
	int l = rand() % (cant_routers+1);
	int arreglo2[2]={k,l};
	vector<int> ip_comp_destino(arreglo2,arreglo2+sizeof(arreglo2)/sizeof(arreglo2[0]));
	Pagina nva_pag(total_pag, rand()%(TAM_MAX_PAG+1-TAM_MIN_PAG)+TAM_MIN_PAG, ip_comp_origen, ip_comp_destino);
	//*computadoras.elemento_pos( cant_comp_por_router * cant_routers-1).enviar_pagina(nva_pag);//Ojo revisar si se envia desde la máquina correcta.
	srand(time(0));
	int m = rand() % (computadoras.tamanio()+1);
	computadoras.elemento_pos(m)->enviar_pagina(nva_pag);
	total_pag++;
}

void Administrador::simular_un_paso(){//recorrer la lista de routers y ejecutar enviar y despues en otro ciclo recibir
	
	if(cant_pasos == 0){//Si este es el primer paso de simulación
		this->calcular_tablas();
		int bandera=cant_comp_por_routers*cant_routers; 
		while(bandera==0){
			this->crear_pagina();//Se crearán tantas páginas como computadoras tiene el sistema.
			bandera--;
		}
	}
	if((cant_pasos % 5) == 0){//Cada 5 pasos de simulación se crea una página.
		this->crear_pagina();
	}
	if((cant_pasos % 30) == 0){//Cada 30 pasos de simulación se recalculan las tablas de enrutamiento.
		int nvo_peso;		
		for(int i=0;i<arcos.tamanio();i++){
			nvo_peso=arcos_originales.elemento_pos(i).peso()/routers.elemento_pos(arcos_originales.elemento_pos(i).destino()).total_paquetes();
			arcos.elemento_pos(i).mod_peso(nvo_peso);
		}
		this->calcular_tablas();
	}

	for(int i=0;i<routers.tamanio();i++){
		routers.elemento_pos(i).enviar_paquetes();
	}
	for(int i=0;i<routers.tamanio();i++){
		routers.elemento_pos(i).recibir_paquetes();
	}
}

Lista<Etiqueta> Administrador::Dijkstra(int nodo_inicio){
	Lista<int> S();
	Lista<int> Q();
	Lista<Etiqueta> etiquetas(); 
//------------INICIALIZACIÓN-------------------------------------
	for(int i=0; i<cant_routers;i++){
		if(i==nodo_inicio){
			etiquetas.agregar(Etiqueta(i,-1,0));
		}
		else{
			etiquetas.agregar(Etiqueta(i,-1,INF));
		}
		Q.agregar(i);
	}
//------------CICLO DE PROCESAMIENTO-------------------------------------
	while(!Q.es_vacia()){
		int nodo_elegido;
		int peso_tray_menor=INF;
		int peso_tray_actual;
		int pos;
		for(int i=0; i<Q.tamanio(); i++){//Para todos las posiciones de la lista de etiquetas señecciono la de menor peso_trayecto.
			peso_tray_actual=etiquetas.elemento_pos(Q.elemento_pos(i)).peso_total();
			if(peso_tray_actual<peso_tray_menor){
				peso_tray_menor=peso_tray_actual;
				nodo_elegido=Q.elemento_pos(i);
				pos=i;
			}
		}
		Q.quitar_nodo_pos(pos);//Con esta acción corta el ciclo while
		S.agregar(nodo_elegido);
		
		Arco* arco_actual;
		int peso_tray_nvo;
		Etiqueta* etiqueta_vieja;
		for(int k=0;k<arcos.tamanio();k++){//para cada uno de los arcos del grafo.
			arco_actual= &arcos.elemento_pos(k);
			if(arco_actual->origen()==nodo_elegido){//Si el arco tiene como orígen el nodo que estamos analizando. Osea corroboro sobre los que son adyancentes al nodo_elegido.
				if(!S.contiene(arco_actual->destino())){//Si el destino de ese arco NO está dentro de los nodos calculados.
					etiqueta_vieja = & etiquetas.elemento_pos(arco_actual->destino());
					peso_tray_nvo=etiquetas.elemento_pos(nodo_elegido).peso_total()+arco_actual->peso();
					if(etiqueta_vieja->peso_total()>peso_tray_nvo){//Si el peso del trayecto de la etiqueta vieja es mayor que el peso del trayecto calculado.
						etiqueta_vieja->mod_peso_trayecto(peso_tray_nvo);
					}
					if(etiqueta_vieja->router_despacho()==-1 || etiqueta_vieja->router_despacho()==nodo_inicio){//Si no se ha asignado el router de despacho a la etiqueta...
						etiqueta_vieja->mod_despacho(etiquetas.elemento_pos(nodo_elegido).router_despacho);//Se le asigna el router de despacho de su predecesor.
					}
				}
			}
		}
	}
}

void Administrador::calcular_tablas(){
	for(int i=0; i<cant_routers; i++){
		routers.obtener_pos(i).actualizar_tabla(Dijkstra(i));
	}
}

void Administrador::leer_archivo(string nombre_archivo){
	std::ifstream archivo_conf = infile(nombre_archivo);
	string linea;
	while(getline(archivo_conf, linea)){
		switch(linea[0]){
		case('#'):
		{
			linea.erase(line.begin());//borra el primer caracter de la línea, en este caso el símbolo #
			std::istringstream iss(line);
			int nro;
			if (!(iss >> nro)) { 
				cout<<"Error en la lectura de la cantidad de routers."<<endl;
				break; 
			} // error
			cant_routers=nro;
			for(int i=0; i<cant_routers; i++){
				Router router_i(i);
				routers.agregar(router_i);
			}
		}
		break;
		case('*'):
		{
			linea.erase(line.begin());
			std::istringstream iss(linea);
			int nro;
			if (!(iss >> nro)) { 
				cout<<"Error en la lectura de la cantidad de computadoras por router."<<endl;
				break; 
			} // error
			cant_comp_por_router=nro;
			for(int i=0; i<cant_routers; i++){
				for(int j=0; j<cant_comp_por_router; j++){
					int arreglo[]={i,j};
					vector<int> ip_comp_i_j(arreglo,arreglo+sizeof(arreglo)/sizeof(arreglo[0]));
					Router *ptr_router_i=&routers.elemento_pos(i);//puntero al router de la posicion i de la lista de routers de la red.
					
					Computadora computadora_i_j(ip_comp_i_j,ptr_router);
					Computadora *ptr_comp_i_j = &computadora_i_j;//puntero a la computadora recien creada
					computadoras.agregar(ptr_comp_i_j);//se agrega el puntero de la computadora recien creada a la lista de computadoras de la red.
					routers.elemento_pos(i).agregar_computadora(ptr_comp_i_j);//se agrega la computadora a la lista de punteros a computadoras del router al que pertenece.
				}
			}
		}	
		break;
		case('@'):
		{
			linea.erase(line.begin());
			std::istringstream iss(linea);
			int origen, destino, ancho_banda;
			if (!(iss >> router_i >> routeer_j >> ancho_banda)) { 
				cout<<"Error en la lectura de la conexion de router."<<endl;
				break; 
			} // error
			//int peso = ancho_banda/routers.elemento_pos(router_j).total_paquetes();//No tiene propósito hacer esto al iniciar la simulación ya que los routers no tienen paquetes que enviar.
			Conexion nva_conexion(origen, destino, ancho_banda);
			Conexion ptr_conexion = &nva_conexion;
			conexiones.agregar(nva_conexion);
			routers.elemento_pos(ptr_conexion->origen()).agregar_conexion_envio(ptr_conexion);
			routers.elemento_pos(ptr_conexion->destino()).agregar_conexion_recepcion(ptr_conexion);
			Arco nvo_arco(router_i,router_j,ancho_banda);
			arcos.agregar(nvo_arco);
		}
		break;
		}
	}
}

/*
void Administrador::crear_conexiones(){
	for(int i=0; i<arcos.tamanio(); i++){
		Conexion nva_conexion(arcos.elemento_pos(i).origen(), arcos.elemento_pos(i).destino(), arcos_originales.elemento_pos(i).peso());
		bool existe=false;

//Aparece el inconveniente de que si dos arcos con los mismos terminales tienen distinto peso se generará una única conexión con
//el ancho de banda del primer arco analizado. OJO! 
		for(int m=0; m<conexiones.tamanio(); m++){
			if(nva_conexion.terminales()==conexiones.elemento_pos(m).terminales())
			existe=true;
		}
		if(!existe){
			conexiones.agregar(nva_conexion);
		}
	}
}*/

void Administrador::dibujar_grafo(){
	
	FILE * f = popen( "date +\"%m-%d_%H-%M-%S\"", "r" );
    if ( f == 0 ) {
        fprintf( stderr, "No se pudo ejecutar \"date\".\n" );
        return 1;
    }
    const int BUFSIZE = 1000;
    char buf[ BUFSIZE ];
    fgets( buf, BUFSIZE,  f );
    cout<<buf<<endl;
    pclose( f );
    /*std::string linea(buf);
    linea[linea.size()-1]='\0';
    std::stringstream flujo_cadenas;
	flujo_cadenas << "grafo_" << linea << ".dot";
	std::string cadena_concat = flujo_cadenas.str();
	cout<<cadena_concat<<endl;
	*/
	buf[strlen(buf)-1]='\0';
	char nombre_archivo[100];
	strcpy(nombre_archivo,"grafo_");
	strcat(nombre_archivo,buf);
	strcat(nombre_archivo,".dot");
	cout<<nombre_archivo<<endl;
	ofstream flujo_salida;
	
	
	flujo_salida.open(nombre_archivo);
	flujo_salida<<"graph G \{"<<endl;
	flujo_salida<<"graph [splines = ortho];"<<endl;
	for(int i=0; i<cant_routers;i++){
		for(int j=0; j<cant_comp_por_router;j++){
		flujo_salida<<i<<" -- m"<<i<<j<<";"<<endl;
		flujo_salida<<"m"<<i<<j<<" [label=\"\",shape=circle,height=0.12,width=0.12,fontsize=1,color=red,style=filled];"<<endl;
		}
		flujo_salida<<i<<" [label="<<"\"R"<<i<<"\""<<",shape=doublecircle,fontsize=10];"<<endl;
	}
	for(int k=0;k<conexiones.tamanio();k++){
		/*set<int>::iterator it_1 = conexiones.elemento_pos(k).terminales().begin();
		set<int>::iterator it_2 = it_1++;
		flujo_salida<<*it_1<<" --> "<<*it_2<<" [color=blue,penwidth=3.0]"<<endl;*/
		flujo_salida<<conexiones.primer_elemento().origen()<<" --> "<<conexiones.primer_elemento().destino()<<" [color=blue,penwidth=3.0]"<<endl;
	}
	flujo_salida<<"}"<<endl;
	
	char compilar_dot[100];
	strcpy(compilar_dot,"dot -Tpng ");
	strcat(compilar_dot,nombre_archivo);
	strcat(compilar_dot," -o ");
	strcat(compilar_dot,buf);
	strcat(compilar_dot,".png");
		
	FILE * f2 = popen( compilar_dot, "r" );
    if ( f2 == 0 ) {
        fprintf( stderr, "No se pudo ejecutar el compilar_dot.\n" );
        return 1;
    }
    char mostrar_png[100];
	strcpy(mostrar_png,"eog ");
	strcat(mostrar_png,buf);
	strcat(mostrar_png,".png");
	
	cout<<mostrar_png<<endl;
	
    int Input[2], Output[2];

	pipe( Input );
	pipe( Output );

	if( fork() ){
		// Este es el proceso padre.
		// Close the reading end of the input pipe.
		close( Input[ 0 ] );
		// Close the writing end of the output pipe
		close( Output[ 1 ] );

		// Here we can interact with the subprocess.  Write to the subprocesses stdin via Input[ 1 ], and read from the subprocesses stdout via Output[ 0 ].
	}
	else{    // We're in the child here.
		close( Input[ 1 ] );
		dup2( Input[ 0 ], STDIN_FILENO );
		close( Output[ 0 ] );
		dup2( Output[ 1 ], STDOUT_FILENO );

		FILE * f3 = popen( mostrar_png, "r" );
		if ( f3 == 0 ) {
			fprintf( stderr, "No se pudo ejecutar mostrar_png.\n" );
			return 1;
		}
	}
}
