import json
import os
import tkinter as tk
from tkinter import messagebox, ttk

ARCHIVO = "personas.json"
FONDO = "#f3f6fb"
TINTA = "#17243a"
AZUL = "#315efb"


def cargar():
    if not os.path.exists(ARCHIVO):
        return []
    try:
        with open(ARCHIVO, "r", encoding="utf-8") as archivo:
            return json.load(archivo)
    except (OSError, json.JSONDecodeError):
        messagebox.showwarning("Datos", "No se pudieron leer los datos. Se iniciará una lista vacía.")
        return []


def guardar(personas):
    try:
        with open(ARCHIVO, "w", encoding="utf-8") as archivo:
            json.dump(personas, archivo, ensure_ascii=False, indent=4)
    except OSError as error:
        messagebox.showerror("Error", f"No se pudieron guardar los datos.\n{error}")


class Agenda:
    def __init__(self, ventana):
        self.ventana = ventana
        self.ventana.title("Agenda | ABM de personas")
        self.ventana.geometry("900x610")
        self.ventana.minsize(760, 530)
        self.ventana.configure(bg=FONDO)
        self.personas = cargar()
        self.campos = {}
        self.crear_interfaz()
        self.refrescar()

    def crear_interfaz(self):
        estilo = ttk.Style()
        estilo.theme_use("clam")
        estilo.configure("Treeview", rowheight=32, font=("Segoe UI", 10), background="white", fieldbackground="white")
        estilo.configure("Treeview.Heading", font=("Segoe UI", 9, "bold"), foreground=TINTA)
        estilo.map("Treeview", background=[("selected", "#dce5ff")], foreground=[("selected", TINTA)])

        contenedor = tk.Frame(self.ventana, bg=FONDO, padx=30, pady=24)
        contenedor.pack(fill="both", expand=True)
        tk.Label(contenedor, text="Agenda de personas", font=("Segoe UI", 23, "bold"), fg=TINTA, bg=FONDO).pack(anchor="w")
        tk.Label(contenedor, text="Administrá tus contactos de forma simple", font=("Segoe UI", 10), fg="#65738a", bg=FONDO).pack(anchor="w", pady=(3, 18))

        tarjeta = tk.Frame(contenedor, bg="white", padx=18, pady=15, highlightbackground="#e2e8f0", highlightthickness=1)
        tarjeta.pack(fill="x", pady=(0, 16))
        for indice, (campo, titulo) in enumerate((("nombre", "Nombre"), ("telefono", "Teléfono"), ("correo", "Correo electrónico"))):
            bloque = tk.Frame(tarjeta, bg="white")
            bloque.grid(row=0, column=indice, sticky="ew", padx=(0 if indice == 0 else 12, 0))
            tk.Label(bloque, text=titulo, font=("Segoe UI", 9, "bold"), fg="#536176", bg="white").pack(anchor="w", pady=(0, 6))
            entrada = ttk.Entry(bloque, font=("Segoe UI", 10))
            entrada.pack(fill="x", ipady=5)
            self.campos[campo] = entrada
            tarjeta.columnconfigure(indice, weight=1)

        acciones = tk.Frame(tarjeta, bg="white")
        acciones.grid(row=1, column=0, columnspan=3, sticky="w", pady=(14, 0))
        self.boton(acciones, "＋  Agregar", self.agregar, AZUL, "white").pack(side="left", padx=(0, 8))
        self.boton(acciones, "Guardar cambios", self.modificar, "#e9efff", AZUL).pack(side="left", padx=(0, 8))
        self.boton(acciones, "Eliminar", self.eliminar, "#fff0f0", "#bd3434").pack(side="left", padx=(0, 8))
        self.boton(acciones, "Limpiar", self.limpiar, "#f0f3f8", "#536176").pack(side="left")

        barra = tk.Frame(contenedor, bg=FONDO)
        barra.pack(fill="x", pady=(0, 8))
        tk.Label(barra, text="Contactos", font=("Segoe UI", 13, "bold"), fg=TINTA, bg=FONDO).pack(side="left")
        self.busqueda = ttk.Entry(barra, width=30, font=("Segoe UI", 10))
        self.busqueda.pack(side="right", ipady=5)
        self.busqueda.bind("<KeyRelease>", lambda _evento: self.refrescar())
        tk.Label(barra, text="Buscar", font=("Segoe UI", 9), fg="#65738a", bg=FONDO).pack(side="right", padx=8)

        tabla_frame = tk.Frame(contenedor, bg="white", highlightbackground="#e2e8f0", highlightthickness=1)
        tabla_frame.pack(fill="both", expand=True)
        columnas = ("id", "nombre", "telefono", "correo")
        self.tabla = ttk.Treeview(tabla_frame, columns=columnas, show="headings", selectmode="browse")
        for columna, titulo, ancho in (("id", "ID", 60), ("nombre", "NOMBRE", 220), ("telefono", "TELÉFONO", 160), ("correo", "CORREO ELECTRÓNICO", 300)):
            self.tabla.heading(columna, text=titulo)
            self.tabla.column(columna, width=ancho, anchor="w")
        scroll = ttk.Scrollbar(tabla_frame, orient="vertical", command=self.tabla.yview)
        self.tabla.configure(yscrollcommand=scroll.set)
        self.tabla.pack(side="left", fill="both", expand=True, padx=8, pady=8)
        scroll.pack(side="right", fill="y", pady=8)
        self.tabla.bind("<<TreeviewSelect>>", self.seleccionar)
        self.estado = tk.Label(contenedor, text="", font=("Segoe UI", 9), fg="#65738a", bg=FONDO)
        self.estado.pack(anchor="w", pady=(9, 0))

    @staticmethod
    def boton(padre, texto, accion, fondo, color):
        return tk.Button(padre, text=texto, command=accion, relief="flat", bd=0, padx=14, pady=8,
                         font=("Segoe UI", 9, "bold"), bg=fondo, fg=color, activebackground=fondo,
                         activeforeground=color, cursor="hand2")

    def valores(self):
        return {campo: entrada.get().strip() for campo, entrada in self.campos.items()}

    def agregar(self):
        datos = self.valores()
        if not datos["nombre"]:
            messagebox.showinfo("Falta el nombre", "Ingresá al menos el nombre de la persona.")
            self.campos["nombre"].focus_set()
            return
        nuevo_id = max((p["id"] for p in self.personas), default=0) + 1
        self.personas.append({"id": nuevo_id, **datos})
        guardar(self.personas)
        self.refrescar()
        self.limpiar()
        self.estado.config(text=f"Contacto agregado · ID {nuevo_id}")

    def modificar(self):
        persona = self.persona_seleccionada()
        if persona is None:
            messagebox.showinfo("Seleccioná un contacto", "Elegí una fila de la tabla para modificarla.")
            return
        datos = self.valores()
        if not datos["nombre"]:
            messagebox.showinfo("Falta el nombre", "El nombre no puede quedar vacío.")
            return
        persona.update(datos)
        guardar(self.personas)
        self.refrescar()
        self.estado.config(text=f"Cambios guardados · ID {persona['id']}")

    def eliminar(self):
        persona = self.persona_seleccionada()
        if persona is None:
            messagebox.showinfo("Seleccioná un contacto", "Elegí una fila de la tabla para eliminarla.")
            return
        if not messagebox.askyesno("Eliminar contacto", f"¿Eliminar a {persona['nombre']}?"):
            return
        self.personas.remove(persona)
        guardar(self.personas)
        self.refrescar()
        self.limpiar()
        self.estado.config(text="Contacto eliminado")

    def seleccionar(self, _evento=None):
        persona = self.persona_seleccionada()
        if persona:
            for campo, entrada in self.campos.items():
                entrada.delete(0, tk.END)
                entrada.insert(0, persona.get(campo, ""))

    def persona_seleccionada(self):
        seleccion = self.tabla.selection()
        if not seleccion:
            return None
        persona_id = int(seleccion[0])
        return next((p for p in self.personas if p["id"] == persona_id), None)

    def refrescar(self):
        if not hasattr(self, "tabla"):
            return
        filtro = self.busqueda.get().strip().lower() if hasattr(self, "busqueda") else ""
        for fila in self.tabla.get_children():
            self.tabla.delete(fila)
        visibles = 0
        for persona in self.personas:
            texto = " ".join(str(persona.get(campo, "")) for campo in ("id", "nombre", "telefono", "correo"))
            if filtro in texto.lower():
                self.tabla.insert("", "end", iid=str(persona["id"]), values=(persona["id"], persona["nombre"], persona["telefono"], persona["correo"]))
                visibles += 1
        self.estado.config(text=f"{visibles} contacto{'s' if visibles != 1 else ''}")

    def limpiar(self):
        for entrada in self.campos.values():
            entrada.delete(0, tk.END)
        self.tabla.selection_remove(self.tabla.selection())
        self.campos["nombre"].focus_set()


if __name__ == "__main__":
    raiz = tk.Tk()
    Agenda(raiz)
    raiz.mainloop()
