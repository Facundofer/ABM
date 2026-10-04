import json
import os

ARCHIVO = "personas.json"


def cargar_personas():
    """Lee los registros guardados; si todavía no hay archivo, inicia vacío."""
    if not os.path.exists(ARCHIVO):
        return []

    try:
        with open(ARCHIVO, "r", encoding="utf-8") as archivo:
            return json.load(archivo)
    except (json.JSONDecodeError, OSError):
        print("No se pudieron leer los datos. Se iniciará una lista vacía.")
        return []


def guardar_personas(personas):
    try:
        with open(ARCHIVO, "w", encoding="utf-8") as archivo:
            json.dump(personas, archivo, ensure_ascii=False, indent=4)
    except OSError:
        print("No se pudieron guardar los datos.")


def pedir_id(mensaje):
    while True:
        try:
            return int(input(mensaje))
        except ValueError:
            print("Ingresá un número válido.")


def buscar_persona(personas, persona_id):
    for persona in personas:
        if persona["id"] == persona_id:
            return persona
    return None


def alta(personas):
    if personas:
        nuevo_id = max(persona["id"] for persona in personas) + 1
    else:
        nuevo_id = 1

    persona = {
        "id": nuevo_id,
        "nombre": input("  Nombre:   ").strip(),
        "telefono": input("  Teléfono: ").strip(),
        "correo": input("  Correo:   ").strip(),
    }
    personas.append(persona)
    guardar_personas(personas)
    print(f"\n  ✓ Persona agregada con el ID {nuevo_id}.")


def listar(personas):
    if not personas:
        print("  Todavía no hay personas cargadas.")
        return

    print(f"  {'ID':<5} {'NOMBRE':<28} {'TELÉFONO':<20} CORREO")
    print("  -----------------------------------------------------------------------------")
    for persona in personas:
        print(
            f"  {persona['id']:<5} {persona['nombre']:<28} "
            f"{persona['telefono']:<20} {persona['correo']}"
        )


def consultar(personas):
    persona_id = pedir_id("  ID a buscar: ")
    persona = buscar_persona(personas, persona_id)
    if persona is None:
        print("  No existe una persona con ese ID.")
        return

    print(
        f"\n  ID:       {persona['id']}\n"
        f"  Nombre:   {persona['nombre']}\n"
        f"  Teléfono: {persona['telefono']}\n"
        f"  Correo:   {persona['correo']}"
    )


def modificar(personas):
    persona_id = pedir_id("  ID a modificar: ")
    persona = buscar_persona(personas, persona_id)
    if persona is None:
        print("  No existe una persona con ese ID.")
        return

    print("  Dejá un campo vacío para conservar su valor actual.")
    for campo, etiqueta in (("nombre", "Nombre"), ("telefono", "Teléfono"), ("correo", "Correo")):
        dato = input(f"  {etiqueta} [{persona[campo]}]: ").strip()
        if dato:
            persona[campo] = dato

    guardar_personas(personas)
    print("  ✓ Datos actualizados.")


def baja(personas):
    persona_id = pedir_id("  ID a eliminar: ")
    persona = buscar_persona(personas, persona_id)
    if persona is None:
        print("  No existe una persona con ese ID.")
        return

    personas.remove(persona)
    guardar_personas(personas)
    print("  ✓ Persona eliminada.")


def mostrar_menu():
    print("\n  ================================================")
    print("              AGENDA | ABM DE PERSONAS")
    print("  ================================================")
    print("   1. Alta de persona")
    print("   2. Listar personas")
    print("   3. Buscar persona")
    print("   4. Modificar persona")
    print("   5. Baja de persona")
    print("   0. Salir\n")


def main():
    personas = cargar_personas()

    while True:
        mostrar_menu()
        opcion = input("  Elegí una opción: ").strip()
        print()

        if opcion == "1":
            alta(personas)
        elif opcion == "2":
            listar(personas)
        elif opcion == "3":
            consultar(personas)
        elif opcion == "4":
            modificar(personas)
        elif opcion == "5":
            baja(personas)
        elif opcion == "0":
            print("  Hasta luego. Los datos quedaron guardados.")
            break
        else:
            print("  Opción fuera del menú. Probá de nuevo.")


if __name__ == "__main__":
    main()
