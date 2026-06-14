from flask import Flask, jsonify, request
from sqlalchemy import create_engine, select, update
from sqlalchemy.orm import declarative_base, sessionmaker
from database import Nauczyciele, Studenci, Terminarz, PBL4, MUP, ARIUS
from datetime import datetime
from zeroconf import ServiceInfo, Zeroconf
import time, socket

def register_mdns_service():
    # Pobieramy lokalny adres IP serwera
    hostname = socket.gethostname()
    ip_address = socket.gethostbyname(hostname)

    # Zeroconf potrzebuje adresu w postaci bajtów
    ip_bytes = socket.inet_aton(ip_address)

    # Definicja serwisu HTTP na porcie 5000
    info = ServiceInfo(
        type_="_http._tcp.local.",
        name="Konfident._http._tcp.local.",  # nazwa widoczna w sieci
        port=5000,
        addresses=[ip_bytes],
        properties={},  # można tu dodać własne parametry
        server=f"Konfident.local."
    )

    zeroconf = Zeroconf()
    zeroconf.register_service(info)
    print(f"Zarejestrowano mDNS jako http://Konfident.local:5000")

    return zeroconf, info

app = Flask(__name__)

Base = declarative_base()

engine = create_engine('sqlite:///konfident.db')
Base.metadata.create_all(engine)

Session = sessionmaker(bind=engine)
session = Session()

model_map = {"PBL4": PBL4,"MUP": MUP,"ARIUS": ARIUS}

@app.route('/get-teacher/<uuid_nauczyciela>')
def get_teacher(uuid_nauczyciela):
    results = session.execute(select(Nauczyciele.imie, Nauczyciele.nazwisko, Nauczyciele.zdjecie).where(Nauczyciele.uuid_nauczyciela == uuid_nauczyciela)).first()
    if results is None:
        return jsonify({"error": f"Nauczyciel o uuid {uuid_nauczyciela} nie istnieje"}), 404
    imie, nazwisko, zdjecie = results
    zdjecie_tab = list(zdjecie)
    tab = []
    tab.append(imie)
    tab.append(nazwisko)
    tab.append(zdjecie_tab)
    return tab

@app.route('/get-lessons/<uuid_nauczyciela>')
def get_lessons(uuid_nauczyciela):
    data = datetime.now()
    results = session.execute(select(Terminarz.id, Terminarz.nazwa_przedmiotu).where(Terminarz.uuid_nauczyciela == uuid_nauczyciela).where(Terminarz.data_start < data).where(Terminarz.data_koniec > data))
    list1 = []
    for przedmiot in results:
        id, nazwa_przedmiotu = przedmiot
        list2 =[]
        list2.append(id)
        list2.append(nazwa_przedmiotu)
        list1.append(list2)
    return list1

@app.route('/get-list/<id>')
def get_list(id):
    results = session.execute(select(Terminarz.nazwa_przedmiotu, Terminarz.id).where(Terminarz.id == id)).first()
    if results is None:
        return jsonify({"error": f"lekcja o id {id} nie istnieje"}), 404
    list1 = []
    nazwa, id = results
    for column in model_map.get(nazwa).__table__.columns:
        if column.name == "id_lekcji":
            continue
        results = session.execute(select(Studenci.uuid_studenta, Studenci.nr_albumu, Studenci.imie, Studenci.nazwisko).where(Studenci.uuid_studenta == column.name[8:])).first()
        uuid_studenta, nr_albumu, imie, nazwisko = results
        list2 = []
        list2.append(uuid_studenta)
        list2.append(nr_albumu)
        list2.append(imie)
        list2.append(nazwisko)
        list1.append(list2)
    return jsonify({"list1": list1})

@app.route('/get-list-all/<id>')
def get_list_all(id):
    results = session.execute(select(Terminarz.nazwa_przedmiotu, Terminarz.id).where(Terminarz.id == id)).first()
    if results is None:
        return jsonify({"error": f"lekcja o id {id} nie istnieje"}), 404
    list1 = []
    nazwa, id = results
    print(nazwa)
    for column in model_map.get(nazwa).__table__.columns:
        if column.name == "id_lekcji":
            continue
        results = session.execute(select(Studenci.uuid_studenta, Studenci.nr_albumu, Studenci.imie, Studenci.nazwisko, Studenci.zdjecie).where(Studenci.uuid_studenta == column.name[8:])).first()
        uuid_studenta, nr_albumu, imie, nazwisko, zdjecie1 = results
        zdjecie = list(zdjecie1)
        list2 = []
        list2.append(uuid_studenta)
        list2.append(nr_albumu)
        list2.append(imie)
        list2.append(nazwisko)
        list2.append(zdjecie)
        list1.append(list2)
    return list1

@app.route('/check-student/<uuid_studenta>')
def check_student(uuid_studenta):
    results = session.execute(select(Studenci.nr_albumu, Studenci.imie, Studenci.nazwisko, Studenci.zdjecie).where(Studenci.uuid_studenta == uuid_studenta)).first()
    if results is None:
        return jsonify({"error": f"Student o uuid {uuid_studenta} nie istnieje"}), 404
    nr_albumu, imie, nazwisko, zdjecie = results
    zdjecie_tab = list(zdjecie)
    lista = []
    lista.append(nr_albumu)
    lista.append(imie)
    lista.append(nazwisko)
    lista.append(zdjecie_tab)
    return lista

@app.route('/send-attendance/<id>/<lista>')
def send_attendance(id,lista):
    results = session.execute(select(Terminarz.nazwa_przedmiotu,Terminarz.id).where(Terminarz.id == id)).first()
    nazwa, x = results
    name = model_map.get(nazwa)
    results = session.execute(select(name.id_lekcji).where(name.id_lekcji == id)).first()
    if results is None:
        new_lesson = name(id_lekcji = id)
        session.add(new_lesson)
        session.commit()
    for column in name.__table__.columns:
        column_name = column.name
        if column_name == "id_lekcji":
            continue
        print(column_name)
        if column_name[8:] in lista:
            print(column_name[8:])
            session.execute(update(name).where(name.id_lekcji == id).values({column_name: True}))
            session.commit()
    return "Pomyślnie zaznaczono obecność na lekcji"

@app.route('/get-date')
def get_date():
    timestamp = str(time.time())
    return timestamp

@app.route('/<path:a>')
def not_addres(**args):     #Funkcja dla złej ścieżki
    return jsonify({"error": f'<h1>To nie jest poprawny adres<h1>'}),404

if __name__ == "__main__":
    zeroconf, info = register_mdns_service()
    try:
        app.run(host="0.0.0.0", port=5000, debug=True)
    finally:
        zeroconf.unregister_service(info)
        zeroconf.close()