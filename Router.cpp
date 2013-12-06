#include "Router.h"

Router::Router(int ip_router)
	:ip(ip_router),computadoras(),tabla_enrutamiento(),organizador_paquetes(ip_router),conexiones(){}

void Router::recibir_pagina(Pagina pagina_recibida){
	for(int i=0;i<pagina_recibida.obtener_tamanio();i++){
		this->organizador_paquetes.agregar_paquete(Paquete(pagina_recibida.obtener_ip_comp_origen(), pagina_recibida.obtener_ip_comp_destino(), pagina_recibida.obtener_id(), pagina_recibida.obtener_tamanio(),i));
	}
}

void Router::recibir_paquetes(){
	this->leer_conexiones();
	
}

void Router::leer_conexiones(){
	for(int i=0; i<conexiones.tamanio(); i++){
		while(!conexiones.elemento_pos(i).conexion_libre()){//Mientras que la conexión en la posición i de la lsita de conexiones no esté libre...
			Paquete paq_leido=conexiones.elemento_pos(i).leer();//esta instruccion terminará por liberar la conexion y finalizar el while.
			bool pag_completa=this->organizador_paquetes.agregar_paquete(paq_leido);
			if(pagina_completa){//si estan todos los paquetes de una página para este router
				Pagina pag_construida = this->construir_pagina(paq_leido);//en la realidad la computadora se encarga de generar la pagina
				this->enviar_pagina(pag_construida);
		}
	}
}

Pagina Router::construir_pagina(Paquete paq_leido){
	return Pagina(paq_leido.obtener_id_pagina(), paq_leido.obtener_tamanio_pagina(), paq_leido.obtener_ip_comp_origen(), paq_leido.obtener_ip_comp_destino());
}//Devuelve una página con la informaxión del paquete paq_leido.

void Router::enviar_pagina(Pagina pag_cons){
	for(int i=0; i<computadoras.tamanio();i++){//recorre la lista de punteros a computadoras y le envpia la página a la maquina apuntada que tiene el mismo ip que el destino de la página.
		if(computadoras.elemento_pos(i)->obtener_ip() == pag_cons.obtener_ip_comp_destino())
				computadoras.elemento_pos(i)->recibir_pagina(pag_cons);
	}
}

void Router::enviar_paquetes(){
	this->cargar_conexiones();
	
}
void Router::cargar_conexiones(){//envia el mensaje pero no espera una respuesta de recepcion exitosa UTP!
	bool conexiones_saturadas=false;
	while(organizador_paquetes.tamanio()!=0 && !conexiones_saturadas){//mientras hayan paquetes en el organizador y las conexiones no esten sat 
		Paquete paq_envio = this->organizador_paquetes.obtener_paquete();//aqui se puede vaciar el organizador_paquetes -->corta el while
		int router_despacho=buscar_en_tabla(paq_envio).router_despacho();
		conexiones_saturadas=true;//inicializa en verdadero asi permite hacer la operacion AND. 
		for(int i=0; i<conexiones.tamanio(); i++){//este ciclo recorre todas las conexiones del router en busca de aquella que lo conecta con el router de despacho para tal paquete paw_envio.
			int bornes[]={this->ip,router_despacho};
			set<int> estos_terminales(bornes,bornes+2);
			if(conexiones.elemento_pos(i).obtener_terminales()==estos_terminales)
				conexiones.elemento_pos(i).cargar(paq_envio);
			conexiones_saturadas=conexiones_saturadas && conexiones.elemento_pos(i).conexion_saturada();//Aqui corta el while si las conexiones estan saturadas.
		}	
	}
}

Etiqueta Router::buscar_en_tabla(Paquete paq){
	for(int i=0;i<tabla_enrutamiento.tamanio();i++){
		if(tabla_enrutamiento.elemento_pos(i).router_destino() == paq.obtener_ip_comp_destino()[0])
			return tabla_enrutamiento.elemento_pos(i);
	}
	assert(0 && "Router::buscar_en_tabla -> \"No se encontró la etiqueta para el destino del paquete.\"");
}

