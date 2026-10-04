#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARCHIVO "personas.dat"
#define MAX_NOMBRE 60
#define MAX_TELEFONO 25
#define MAX_CORREO 80
#define ID_NOMBRE 101
#define ID_TELEFONO 102
#define ID_CORREO 103
#define ID_BUSCAR 104
#define ID_LISTA 105

typedef struct Persona {
    int id;
    char nombre[MAX_NOMBRE];
    char telefono[MAX_TELEFONO];
    char correo[MAX_CORREO];
    struct Persona *siguiente;
} Persona;

static Persona *personas = NULL;
static HWND campoNombre, campoTelefono, campoCorreo, campoBuscar, lista;
static HFONT fuente;

static void guardar(void) {
    FILE *archivo = fopen(ARCHIVO, "wb");
    Persona *actual = personas;
    if (archivo == NULL) return;
    while (actual != NULL) {
        fwrite(&actual->id, sizeof(actual->id), 1, archivo);
        fwrite(actual->nombre, sizeof(actual->nombre), 1, archivo);
        fwrite(actual->telefono, sizeof(actual->telefono), 1, archivo);
        fwrite(actual->correo, sizeof(actual->correo), 1, archivo);
        actual = actual->siguiente;
    }
    fclose(archivo);
}

static void cargar(void) {
    FILE *archivo = fopen(ARCHIVO, "rb");
    Persona *ultimo = NULL;
    Persona temporal;
    if (archivo == NULL) return;
    while (fread(&temporal.id, sizeof(temporal.id), 1, archivo) == 1 &&
           fread(temporal.nombre, sizeof(temporal.nombre), 1, archivo) == 1 &&
           fread(temporal.telefono, sizeof(temporal.telefono), 1, archivo) == 1 &&
           fread(temporal.correo, sizeof(temporal.correo), 1, archivo) == 1) {
        Persona *nuevo = malloc(sizeof(Persona));
        if (nuevo == NULL) break;
        *nuevo = temporal;
        nuevo->siguiente = NULL;
        if (personas == NULL) personas = nuevo;
        else ultimo->siguiente = nuevo;
        ultimo = nuevo;
    }
    fclose(archivo);
}

static int proximoId(void) {
    int mayor = 0;
    Persona *actual = personas;
    while (actual != NULL) {
        if (actual->id > mayor) mayor = actual->id;
        actual = actual->siguiente;
    }
    return mayor + 1;
}

static Persona *personaPorIndice(int indice) {
    Persona *actual = personas;
    while (actual != NULL && indice > 0) {
        actual = actual->siguiente;
        indice--;
    }
    return actual;
}

static Persona *personaPorId(int id) {
    Persona *actual = personas;
    while (actual != NULL) {
        if (actual->id == id) return actual;
        actual = actual->siguiente;
    }
    return NULL;
}

static void actualizarLista(void) {
    Persona *actual = personas;
    char fila[220];
    SendMessageA(lista, LB_RESETCONTENT, 0, 0);
    while (actual != NULL) {
        snprintf(fila, sizeof(fila), "%d   |   %-26s | %-18s | %s", actual->id,
                 actual->nombre, actual->telefono, actual->correo);
        SendMessageA(lista, LB_ADDSTRING, 0, (LPARAM)fila);
        actual = actual->siguiente;
    }
}

static void leerCampo(HWND control, char *destino, int tamanio) {
    GetWindowTextA(control, destino, tamanio);
}

static void limpiarCampos(void) {
    SetWindowTextA(campoNombre, "");
    SetWindowTextA(campoTelefono, "");
    SetWindowTextA(campoCorreo, "");
    SetFocus(campoNombre);
}

static void mostrarPersona(Persona *persona) {
    char id[20];
    if (persona == NULL) return;
    snprintf(id, sizeof(id), "%d", persona->id);
    SetWindowTextA(campoBuscar, id);
    SetWindowTextA(campoNombre, persona->nombre);
    SetWindowTextA(campoTelefono, persona->telefono);
    SetWindowTextA(campoCorreo, persona->correo);
}

static void alta(HWND ventana) {
    Persona *nuevo = malloc(sizeof(Persona));
    Persona *ultimo = personas;
    if (nuevo == NULL) {
        MessageBoxA(ventana, "No hay memoria disponible.", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    leerCampo(campoNombre, nuevo->nombre, sizeof(nuevo->nombre));
    leerCampo(campoTelefono, nuevo->telefono, sizeof(nuevo->telefono));
    leerCampo(campoCorreo, nuevo->correo, sizeof(nuevo->correo));
    if (nuevo->nombre[0] == '\0') {
        free(nuevo);
        MessageBoxA(ventana, "Ingresá al menos el nombre.", "Falta un dato", MB_OK | MB_ICONINFORMATION);
        SetFocus(campoNombre);
        return;
    }
    nuevo->id = proximoId();
    nuevo->siguiente = NULL;
    if (personas == NULL) personas = nuevo;
    else {
        while (ultimo->siguiente != NULL) ultimo = ultimo->siguiente;
        ultimo->siguiente = nuevo;
    }
    guardar();
    actualizarLista();
    limpiarCampos();
}

static void buscar(HWND ventana) {
    char texto[32];
    leerCampo(campoBuscar, texto, sizeof(texto));
    if (texto[0] == '\0') {
        MessageBoxA(ventana, "Ingresá el ID que querés buscar.", "Buscar", MB_OK | MB_ICONINFORMATION);
        return;
    }
    Persona *persona = personaPorId(atoi(texto));
    if (persona == NULL) MessageBoxA(ventana, "No se encontró ese ID.", "Buscar", MB_OK | MB_ICONINFORMATION);
    else mostrarPersona(persona);
}

static void modificar(HWND ventana) {
    char texto[32];
    leerCampo(campoBuscar, texto, sizeof(texto));
    Persona *persona = personaPorId(atoi(texto));
    if (persona == NULL) {
        MessageBoxA(ventana, "Seleccioná una persona o buscala por ID.", "Modificar", MB_OK | MB_ICONINFORMATION);
        return;
    }
    leerCampo(campoNombre, persona->nombre, sizeof(persona->nombre));
    leerCampo(campoTelefono, persona->telefono, sizeof(persona->telefono));
    leerCampo(campoCorreo, persona->correo, sizeof(persona->correo));
    if (persona->nombre[0] == '\0') {
        MessageBoxA(ventana, "El nombre no puede quedar vacío.", "Falta un dato", MB_OK | MB_ICONINFORMATION);
        return;
    }
    guardar();
    actualizarLista();
}

static void baja(HWND ventana) {
    char texto[32];
    leerCampo(campoBuscar, texto, sizeof(texto));
    int id = atoi(texto);
    Persona *actual = personas, *anterior = NULL;
    while (actual != NULL && actual->id != id) {
        anterior = actual;
        actual = actual->siguiente;
    }
    if (actual == NULL) {
        MessageBoxA(ventana, "Seleccioná una persona o buscala por ID.", "Eliminar", MB_OK | MB_ICONINFORMATION);
        return;
    }
    char pregunta[160];
    snprintf(pregunta, sizeof(pregunta), "¿Querés eliminar a %s?", actual->nombre);
    if (MessageBoxA(ventana, pregunta, "Confirmar eliminación", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    if (anterior == NULL) personas = actual->siguiente;
    else anterior->siguiente = actual->siguiente;
    free(actual);
    guardar();
    actualizarLista();
    limpiarCampos();
    SetWindowTextA(campoBuscar, "");
}

static HWND crearControl(HWND ventana, const char *clase, const char *texto, DWORD estilo,
                         int x, int y, int ancho, int alto, int id) {
    HWND control = CreateWindowExA(0, clase, texto, WS_CHILD | WS_VISIBLE | estilo,
        x, y, ancho, alto, ventana, (HMENU)(INT_PTR)id, GetModuleHandleA(NULL), NULL);
    if (fuente != NULL) SendMessageA(control, WM_SETFONT, (WPARAM)fuente, TRUE);
    return control;
}

static void crearInterfaz(HWND ventana) {
    fuente = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    crearControl(ventana, "STATIC", "AGENDA DE PERSONAS", SS_LEFT, 28, 20, 420, 38, 0);
    crearControl(ventana, "STATIC", "ABM sencillo · tus contactos, en un solo lugar", SS_LEFT, 30, 55, 450, 25, 0);
    crearControl(ventana, "STATIC", "Nombre", SS_LEFT, 30, 102, 180, 24, 0);
    crearControl(ventana, "STATIC", "Teléfono", SS_LEFT, 235, 102, 180, 24, 0);
    crearControl(ventana, "STATIC", "Correo electrónico", SS_LEFT, 440, 102, 280, 24, 0);
    campoNombre = crearControl(ventana, "EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 30, 128, 190, 30, ID_NOMBRE);
    campoTelefono = crearControl(ventana, "EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 235, 128, 190, 30, ID_TELEFONO);
    campoCorreo = crearControl(ventana, "EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 440, 128, 300, 30, ID_CORREO);
    crearControl(ventana, "BUTTON", "Agregar", BS_PUSHBUTTON, 30, 177, 125, 34, 1);
    crearControl(ventana, "BUTTON", "Guardar cambios", BS_PUSHBUTTON, 165, 177, 150, 34, 2);
    crearControl(ventana, "BUTTON", "Eliminar", BS_PUSHBUTTON, 325, 177, 125, 34, 3);
    crearControl(ventana, "BUTTON", "Limpiar", BS_PUSHBUTTON, 460, 177, 125, 34, 4);
    crearControl(ventana, "STATIC", "Buscar por ID", SS_LEFT, 30, 235, 110, 25, 0);
    campoBuscar = crearControl(ventana, "EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 140, 229, 105, 30, ID_BUSCAR);
    crearControl(ventana, "BUTTON", "Buscar", BS_PUSHBUTTON, 255, 228, 90, 32, 5);
    crearControl(ventana, "STATIC", "CONTACTOS", SS_LEFT, 30, 282, 180, 25, 0);
    lista = crearControl(ventana, "LISTBOX", "", WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                         30, 312, 710, 260, ID_LISTA);
}

static LRESULT CALLBACK procedimiento(HWND ventana, UINT mensaje, WPARAM wParam, LPARAM lParam) {
    (void)lParam;
    switch (mensaje) {
        case WM_CREATE:
            crearInterfaz(ventana);
            cargar();
            actualizarLista();
            return 0;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1: alta(ventana); return 0;
                case 2: modificar(ventana); return 0;
                case 3: baja(ventana); return 0;
                case 4: limpiarCampos(); SetWindowTextA(campoBuscar, ""); return 0;
                case 5: buscar(ventana); return 0;
                case ID_LISTA:
                    if (HIWORD(wParam) == LBN_SELCHANGE) {
                        int indice = (int)SendMessageA(lista, LB_GETCURSEL, 0, 0);
                        mostrarPersona(personaPorIndice(indice));
                    }
                    return 0;
            }
            break;
        case WM_DESTROY: {
            Persona *actual = personas;
            while (actual != NULL) {
                Persona *borrar = actual;
                actual = actual->siguiente;
                free(borrar);
            }
            if (fuente != NULL) DeleteObject(fuente);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(ventana, mensaje, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instancia, HINSTANCE anterior, LPSTR argumentos, int mostrar) {
    (void)anterior; (void)argumentos;
    const char CLASE[] = "AgendaABMWindow";
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = procedimiento;
    wc.hInstance = instancia;
    wc.lpszClassName = CLASE;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    if (!RegisterClassA(&wc)) return 1;
    HWND ventana = CreateWindowExA(0, CLASE, "Agenda | ABM de personas", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 650, NULL, NULL, instancia, NULL);
    if (ventana == NULL) return 1;
    ShowWindow(ventana, mostrar);
    UpdateWindow(ventana);
    MSG mensaje;
    while (GetMessageA(&mensaje, NULL, 0, 0) > 0) {
        TranslateMessage(&mensaje);
        DispatchMessageA(&mensaje);
    }
    return (int)mensaje.wParam;
}
