#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARCHIVO "personas.dat"
#define MAX_NOMBRE 60
#define MAX_TELEFONO 25
#define MAX_CORREO 80

typedef struct Persona {
    int id;
    char nombre[MAX_NOMBRE];
    char telefono[MAX_TELEFONO];
    char correo[MAX_CORREO];
    struct Persona *siguiente;
} Persona;

void leerTexto(const char *mensaje, char *texto, int tamanio) {
    printf("%s", mensaje);
    if (fgets(texto, tamanio, stdin) != NULL) {
        texto[strcspn(texto, "\n")] = '\0';
    }
}

int leerEntero(const char *mensaje) {
    char linea[32];
    int numero;
    char extra;
    while (1) {
        leerTexto(mensaje, linea, sizeof(linea));
        if (sscanf(linea, "%d %c", &numero, &extra) == 1) return numero;
        printf("  Ingresá un número válido.\n");
    }
}

Persona *buscar(Persona *lista, int id) {
    while (lista != NULL) {
        if (lista->id == id) return lista;
        lista = lista->siguiente;
    }
    return NULL;
}

void guardar(Persona *lista) {
    FILE *archivo = fopen(ARCHIVO, "wb");
    if (archivo == NULL) {
        printf("  No se pudo guardar el archivo.\n");
        return;
    }
    while (lista != NULL) {
        /* Guardamos solo los datos: el puntero siguiente no se persiste. */
        fwrite(&lista->id, sizeof(lista->id), 1, archivo);
        fwrite(lista->nombre, sizeof(lista->nombre), 1, archivo);
        fwrite(lista->telefono, sizeof(lista->telefono), 1, archivo);
        fwrite(lista->correo, sizeof(lista->correo), 1, archivo);
        lista = lista->siguiente;
    }
    fclose(archivo);
}

Persona *cargar(void) {
    FILE *archivo = fopen(ARCHIVO, "rb");
    Persona *lista = NULL;
    Persona *ultimo = NULL;
    Persona temporal;
    if (archivo == NULL) return NULL;
    while (fread(&temporal.id, sizeof(temporal.id), 1, archivo) == 1 &&
           fread(temporal.nombre, sizeof(temporal.nombre), 1, archivo) == 1 &&
           fread(temporal.telefono, sizeof(temporal.telefono), 1, archivo) == 1 &&
           fread(temporal.correo, sizeof(temporal.correo), 1, archivo) == 1) {
        Persona *nuevo = malloc(sizeof(Persona));
        if (nuevo == NULL) break;
        *nuevo = temporal;
        nuevo->siguiente = NULL;
        if (lista == NULL) lista = nuevo;
        else ultimo->siguiente = nuevo;
        ultimo = nuevo;
    }
    fclose(archivo);
    return lista;
}

int proximoId(Persona *lista) {
    int mayor = 0;
    while (lista != NULL) {
        if (lista->id > mayor) mayor = lista->id;
        lista = lista->siguiente;
    }
    return mayor + 1;
}

void alta(Persona **lista) {
    Persona *nuevo = malloc(sizeof(Persona));
    Persona *ultimo = *lista;
    if (nuevo == NULL) {
        printf("  No hay memoria disponible.\n");
        return;
    }
    nuevo->id = proximoId(*lista);
    leerTexto("  Nombre:   ", nuevo->nombre, sizeof(nuevo->nombre));
    leerTexto("  Teléfono: ", nuevo->telefono, sizeof(nuevo->telefono));
    leerTexto("  Correo:   ", nuevo->correo, sizeof(nuevo->correo));
    nuevo->siguiente = NULL;
    if (*lista == NULL) *lista = nuevo;
    else {
        while (ultimo->siguiente != NULL) ultimo = ultimo->siguiente;
        ultimo->siguiente = nuevo;
    }
    guardar(*lista);
    printf("\n  ✓ Persona agregada con el ID %d.\n", nuevo->id);
}

void listar(Persona *lista) {
    if (lista == NULL) {
        printf("  Todavía no hay personas cargadas.\n");
        return;
    }
    printf("  %-5s %-28s %-20s %s\n", "ID", "NOMBRE", "TELÉFONO", "CORREO");
    printf("  -----------------------------------------------------------------------------\n");
    while (lista != NULL) {
        printf("  %-5d %-28s %-20s %s\n", lista->id, lista->nombre, lista->telefono, lista->correo);
        lista = lista->siguiente;
    }
}

void consultar(Persona *lista) {
    int id = leerEntero("  ID a buscar: ");
    Persona *persona = buscar(lista, id);
    if (persona == NULL) printf("  No existe una persona con ese ID.\n");
    else {
        printf("\n  ID:       %d\n  Nombre:   %s\n  Teléfono: %s\n  Correo:   %s\n",
               persona->id, persona->nombre, persona->telefono, persona->correo);
    }
}

void modificar(Persona *lista) {
    int id = leerEntero("  ID a modificar: ");
    Persona *persona = buscar(lista, id);
    if (persona == NULL) {
        printf("  No existe una persona con ese ID.\n");
        return;
    }
    printf("  Dejá un campo vacío para conservar su valor actual.\n");
    char dato[MAX_CORREO];
    leerTexto("  Nombre [actual]:   ", dato, sizeof(dato));
    if (dato[0] != '\0') snprintf(persona->nombre, sizeof(persona->nombre), "%s", dato);
    leerTexto("  Teléfono [actual]: ", dato, sizeof(dato));
    if (dato[0] != '\0') snprintf(persona->telefono, sizeof(persona->telefono), "%s", dato);
    leerTexto("  Correo [actual]:   ", dato, sizeof(dato));
    if (dato[0] != '\0') snprintf(persona->correo, sizeof(persona->correo), "%s", dato);
    guardar(lista);
    printf("  ✓ Datos actualizados.\n");
}

void baja(Persona **lista) {
    int id = leerEntero("  ID a eliminar: ");
    Persona *actual = *lista;
    Persona *anterior = NULL;
    while (actual != NULL && actual->id != id) {
        anterior = actual;
        actual = actual->siguiente;
    }
    if (actual == NULL) {
        printf("  No existe una persona con ese ID.\n");
        return;
    }
    if (anterior == NULL) *lista = actual->siguiente;
    else anterior->siguiente = actual->siguiente;
    free(actual);
    guardar(*lista);
    printf("  ✓ Persona eliminada.\n");
}

void liberar(Persona *lista) {
    while (lista != NULL) {
        Persona *borrar = lista;
        lista = lista->siguiente;
        free(borrar);
    }
}

int main(void) {
    Persona *personas = cargar();
    int opcion;
    do {
        printf("\n  ================================================\n");
        printf("              AGENDA | ABM DE PERSONAS\n");
        printf("  ================================================\n");
        printf("   1. Alta de persona\n");
        printf("   2. Listar personas\n");
        printf("   3. Buscar persona\n");
        printf("   4. Modificar persona\n");
        printf("   5. Baja de persona\n");
        printf("   0. Salir\n\n");
        opcion = leerEntero("  Elegí una opción: ");
        printf("\n");
        switch (opcion) {
            case 1: alta(&personas); break;
            case 2: listar(personas); break;
            case 3: consultar(personas); break;
            case 4: modificar(personas); break;
            case 5: baja(&personas); break;
            case 0: printf("  Hasta luego. Los datos quedaron guardados.\n"); break;
            default: printf("  Opción fuera del menú. Probá de nuevo.\n");
        }
    } while (opcion != 0);
    liberar(personas);
    return 0;
}
