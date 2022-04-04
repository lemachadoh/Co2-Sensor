//logger dos dados obtidos através do arduino

from datetime import datetime
from time import time
import serial

porta = "\\.\COM5"
baud = 115200

ser = serial.Serial(porta,baud)
ser.flushInput()

print("Abrindo serial.")

dados = 34560; i = 0

while i <= dados:
    data = str(ser.readline())
    objeto_hoje = datetime.today()
    dia = str(objeto_hoje.day)
    mes = str(objeto_hoje.month)
    ano = str(objeto_hoje.year)
    horario = str(objeto_hoje.strftime("%X"))
    file = open("novus_CCS811.csv","a")
    file.write(data)
    file.write(",")
    file.write(dia + "/" + mes + "/" + ano + " " + horario)
    file.write("\n")
    i=i+1

file.close()
ser.close()
