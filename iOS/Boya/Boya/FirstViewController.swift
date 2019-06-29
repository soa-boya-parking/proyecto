//
//  FirstViewController.swift
//  Boya
//
//  Created by Mac on 01/05/2019.
//  Copyright © 2019 Reflejo. All rights reserved.
//

import UIKit
import CoreBluetooth // Para el bluetooth.
import CoreMotion // Para el sensor de movimiento.
import Foundation
import AVFoundation // Para el audio.

//Estos valores estan definidos en el ESP32.
let kBLEService_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
let kBLE_Characteristic_uuid_Rx = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
let kBLE_Characteristic_uuid_Tx = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

let BLEService_UUID = CBUUID(string: kBLEService_UUID)
let BLE_Characteristic_uuid_Tx = CBUUID(string: kBLE_Characteristic_uuid_Tx)//(Property = Write without response)
let BLE_Characteristic_uuid_Rx = CBUUID(string: kBLE_Characteristic_uuid_Rx)// (Property = Read/Notify)

public protocol BLEDelegate
{
    func bleDidUpdateState()
    func bleDidConnectToPeripheral()
    func bleDidDisconenctFromPeripheral()
    func bleDidReceiveData(data: Data?)
}

class FirstViewController: UIViewController, CBCentralManagerDelegate, CBPeripheralDelegate
{
    public var delegate: BLEDelegate?
    
    public var centralManager : CBCentralManager!
    public var esp32 : CBPeripheral!
    public var characteristics = [String : CBCharacteristic]() // Es una variable tipo diccionario.
    
    //Variables estaticas que son compartidas con la vista Secundaria (De configuracion)
    public static var esp32Shared : CBPeripheral!
    public static var characteristicsShared = [String : CBCharacteristic]()
    
    var characteristicASCIIValue = NSString()
    
    //Manager de Movimiento, agrupa varios sensores de la categoria movimiento.
    let motionManager = CMMotionManager()
    
    //Elementos de la vista.
    @IBOutlet var cloroSelect: UISegmentedControl!
    @IBOutlet var cerrarSelect: UISegmentedControl!
    @IBOutlet var temperaturaLabel: UILabel!
    @IBOutlet var opacidadLabel: UILabel!
    @IBOutlet var ColorLabel: UILabel!
    @IBOutlet var LluviaLabel: UILabel!
    @IBOutlet var HoraInput: UITextField!
    @IBOutlet var capacidadInput: UITextField!
    
    //Action de programar hora.
    @IBAction func guardarHoraAction(_ sender: Any)
    {
        var hora = "9;"
        hora += HoraInput.text!.replacingOccurrences(of: ":", with: ";")
        hora+=";"
        hora+=capacidadInput.text!
        let data: Data = hora.data(using: String.Encoding.utf8)!
        self.esp32.writeValue(data, for: Array(characteristics)[0].value, type: CBCharacteristicWriteType.withResponse)
    }
    
    //Action de dispensar cloro.
    @IBAction func dispensarCloroAction(_ sender: Any)
    {
        var estado = "1;OFF";
        if(cloroSelect.selectedSegmentIndex == 0)
        {
            estado = "1;ON";
        }
        let data: Data = estado.data(using: String.Encoding.utf8)!
        //Array(characteristics)[1].value el Tx esta almacenado en un diccionario, casteo a un array, y es la segunda posicion.
        self.esp32.writeValue(data, for: Array(characteristics)[0].value, type: CBCharacteristicWriteType.withResponse)
    }
    
    //Action de cerrar pileta.
    @IBAction func cerrarPiletaAction(_ sender: Any)
    {
        var estado = "2;OFF";
        if(cloroSelect.selectedSegmentIndex == 0)
        {
            estado = "2;ON";
        }
        let data: Data = estado.data(using: String.Encoding.utf8)!
        //Array(characteristics)[1].value el Tx esta almacenado en un diccionario, casteo a un array, y es la segunda posicion.
        self.esp32.writeValue(data, for: Array(characteristics)[0].value, type: CBCharacteristicWriteType.withResponse)
    }
    
    //Esta funcion es invocada cuando el dispositivo es conectado, es decir pasa a estado 2.
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral)
    {
        print("Conectado")
        peripheral.discoverServices(nil)
    }
    
    //Esta funcion es llamada cuando se intenta hacer un "discoverServices" del dispositivo.
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        
        if error != nil
        {
            print("[ERROR] Error descubriendo servicios. \(error!)")
            return
        }
        
        print("[DEBUG] Servicios encontrados para el dispositivo: \(peripheral.identifier.uuidString)")
        
        for service in peripheral.services!
        {
            let theCharacteristics = [BLE_Characteristic_uuid_Rx, BLE_Characteristic_uuid_Tx]
            peripheral.discoverCharacteristics(theCharacteristics, for: service)
        }
    }
    
    //Funcion que es invocada cuando se "discoverCharacteristics" del dispositivo.
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        
        if error != nil
        {
            print("[ERROR] Error descubriendo caracteristicas. \(error!)")
            return
        }
        
        print("[DEBUG] Caracteristicas descubiertas para el dispositivo: \(peripheral.identifier.uuidString)")
        
        for characteristic in service.characteristics!
        {
            self.characteristics[characteristic.uuid.uuidString] = characteristic
        }
        
        //Cuando descubro las caracteristicas del dispositivo a la vez activo las notificaciones. Es decir lo que me manda el ESP32.
        enableNotifications(enable: true)
        FirstViewController.characteristicsShared = self.characteristics
    }
    
    //Funcion que determina como se van a realizar la lectura de datos provenientes del ESP32.
    func read()
    {
        guard let lectura = self.characteristics[kBLE_Characteristic_uuid_Tx] else { return }
        self.esp32?.readValue(for: lectura)
    }
    
    //Esta funcion es invocada cada vez que el ESP32 envia "algo"
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?)
    {
        if error != nil
        {
            print("[ERROR] Error actualizando valor. \(error!)")
            return
        }
        
        //Si recibo algo proveniente del ESP32, porque podria estar conectado a mas cosas, y recibir de distintos.
        if characteristic.uuid.uuidString == kBLE_Characteristic_uuid_Tx
        {
            self.delegate?.bleDidReceiveData(data: characteristic.value)
            let recibido = [UInt8](characteristic.value!)
            let cadenaBytetoString = String(bytes: recibido, encoding: .utf8)
            //print(cadenaBytetoString!)
            let datosCorrectos = cadenaBytetoString!.components(separatedBy: ";")
            self.temperaturaLabel.text = datosCorrectos[0] + "°C"
            self.opacidadLabel.text = datosCorrectos[1]
            self.LluviaLabel.text = datosCorrectos[2]
            if(datosCorrectos[3].contains("ALERTA"))
            {
                let alertController = UIAlertController(title: "Resultado", message:
                    String(describing: "ALGO ENTRO A LA PILETA"), preferredStyle: .alert)
                alertController.addAction(UIAlertAction(title: "Aceptar", style: .default))
                
                self.present(alertController, animated: true, completion: nil)
            }
            self.ColorLabel.text = datosCorrectos[4]
        }
    }
    
    //Esta funcion activa las "notificaciones" es decir la recepcion de los datos provenientes del ESP32.
    public func enableNotifications(enable: Bool)
    {
        guard let char = self.characteristics[kBLE_Characteristic_uuid_Tx] else { return }
        self.esp32?.setNotifyValue(enable, for: char)
    }
    
    //Esta funcion que es invocada cuando se produce algun cambio de estado en el CBCentralManager
    func centralManagerDidUpdateState(_ central: CBCentralManager)
    {
        if central.state == .poweredOn
        {
            print("Bluetooth activado")
            //Escaneo los dispositivos.
            self.centralManager.scanForPeripherals(withServices: nil, options: nil)
        }
        else
        {
            print("Bluetooth desactivado")
        }
    }

    //Funcion formato objetiveC que es lanzada cuando el sensor de proximidad cambia su valor.
    @objc func proximityChanged(_ notification: Notification) {
        if let device = notification.object as? UIDevice
        {
            if(UIDevice.current.proximityState == true)
            {
                print("Tapado")
                AudioServicesPlayAlertSound(SystemSoundID(1322))
            }
            else
            {
                print("Libre`")
                AudioServicesPlayAlertSound(SystemSoundID(1000))
            }
        }
    }
    
    //Esta funcion es llamada cuando la vista es cargada.
    override func viewDidLoad()
    {
        //Activar el sensor de proximidad.
        UIDevice.current.isProximityMonitoringEnabled = true
        //Imprimo el estado del sensor de proximidad.
        print("enabled: \(UIDevice.current.isProximityMonitoringEnabled)")
        //Anado un observer al estado del sensor de actividad, que se activa cuando cambia el estado y lanza una funcion.
        NotificationCenter.default.addObserver(self, selector: #selector(self.proximityChanged), name: UIDevice.proximityStateDidChangeNotification, object: UIDevice.current)
        // Adaptación de la pantalla al mostrar ú ocultar el teclado.
        NotificationCenter.default.addObserver(self, selector: #selector(keyboardWillShow), name: UIResponder.keyboardWillShowNotification, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(keyboardWillHide), name: UIResponder.keyboardWillHideNotification, object: nil)
        super.viewDidLoad()
        
        self.setupHideKeyboardOnTap()
        
        //Se define que por defecto los botones empiezan en OFF.
        self.cloroSelect.selectedSegmentIndex = 1
        self.cerrarSelect.selectedSegmentIndex = 1
        //Se inicia el manager que controla el bluetooth.
        self.centralManager = CBCentralManager(delegate: self, queue: nil)
        //Se empieza a recibir datos provenientes del acelerometro.
        self.startAccelerometers()
    }
    
    //Esta funcion es invocada cuando se escanean los dispositivos.
    public func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber)
    {
        //Si encontre el ESP32.
        if(peripheral.name == "ESP32")
        {
            //Print de debug.
            print("\nNombre : \(peripheral.name ?? "(No name)")")
            print("Señal(RSSI) : \(RSSI)")
            for ad in advertisementData
            {
                print("Data : \(ad)")
            }
            //Mantengo una referencia FUERTE al dispositivo.
            self.esp32 = peripheral
            //Detengo la busqueda.
            self.centralManager.stopScan()
            //Seteo el delegado.
            self.esp32.delegate = self
            //Empiezo la conexion, debe estar tambien el centralManager.connect de arriba.
            self.centralManager.connect(esp32, options: nil)
            
            FirstViewController.esp32Shared = self.esp32
        }
    }
    
    //Esta funcion administra el sensor del acelerometro, al girar el celular a la izquierda o hacia la derecha se envia la orden de cerrar o abrir la pileta.
    func startAccelerometers()
    {
        //Empiezo con un valor por defecto.
        var antiguoX = 0.5
        //Si el acelerometro esta disponible.
        if motionManager.isAccelerometerAvailable
        {
            //Cada 1.5 segundos voy a leer los datos del acelerometro.
            motionManager.accelerometerUpdateInterval = 1.5
            motionManager.startAccelerometerUpdates(to: OperationQueue.main)
            { (data, error) in
                //Obtengo la medicion actual del acelerometro sobre el eje X.
                let actualX = data!.acceleration.x
                //Forma de detectar el shake mediante variaciones con respecto de la medicion actual a la medicion anterior.
                if(abs(actualX-antiguoX) > 0.70)
                {
                    if(antiguoX > actualX)
                    {
                        let estado = "2;ON";
                        let data: Data = estado.data(using: String.Encoding.utf8)!
                        self.esp32.writeValue(data, for: Array(self.characteristics)[0].value, type: CBCharacteristicWriteType.withResponse)
                        self.cerrarSelect.selectedSegmentIndex = 0
                    }
                    else
                    {
                        let estado = "2;OFF";
                        let data: Data = estado.data(using: String.Encoding.utf8)!
                        self.esp32.writeValue(data, for: Array(self.characteristics)[0].value, type: CBCharacteristicWriteType.withResponse)
                        self.cerrarSelect.selectedSegmentIndex = 1
                    }
                }
                //Por ultimo el valor actual, va a ser el valor anriguo para la proxima vez que vuelva entrar a la funcion.
                antiguoX = actualX
            }
        }
    }
    
    // Adaptación de la pantalla al mostrar ú ocultar el teclado.
    func setupHideKeyboardOnTap()
    {
        self.view.addGestureRecognizer(self.endEditingRecognizer())
        self.navigationController?.navigationBar.addGestureRecognizer(self.endEditingRecognizer())
    }
    
    // Adaptación de la pantalla al mostrar ú ocultar el teclado.
    private func endEditingRecognizer() -> UIGestureRecognizer
    {
        let tap = UITapGestureRecognizer(target: self.view, action: #selector(self.view.endEditing(_:)))
        tap.cancelsTouchesInView = false
        return tap
    }
    // Adaptación de la pantalla al mostrar ú ocultar el teclado.
    @objc func keyboardWillShow(notification: NSNotification)
    {
        if let keyboardSize = (notification.userInfo?[UIResponder.keyboardFrameBeginUserInfoKey] as? NSValue)?.cgRectValue
        {
            if self.view.frame.origin.y == 0
            {
                self.view.frame.origin.y -= 0/*keyboardSize.height*/
            }
        }
    }
    // Adaptación de la pantalla al mostrar ú ocultar el teclado.
    @objc func keyboardWillHide(notification: NSNotification)
    {
        if self.view.frame.origin.y != 0
        {
            self.view.frame.origin.y = 0
        }
    }


}

