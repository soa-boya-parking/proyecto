//
//  SecondViewController.swift
//  Boya
//
//  Created by Mac on 01/05/2019.
//  Copyright © 2019 Reflejo. All rights reserved.
//

import UIKit
import MapKit
import CoreBluetooth
import Foundation

class SecondViewController: UIViewController, CLLocationManagerDelegate {

    //Elementos de la vista.
	@IBOutlet var ssidText: UITextField!
	@IBOutlet var passwordText: UITextField!
	@IBOutlet var ubicacionText: UITextField!
    @IBOutlet var ubicacionSwitch: UISwitch!
    @IBOutlet var horaSwitch: UISwitch!
    
    //Manager del sensorGPS.
    var locationManager = CLLocationManager()
    
    //Variables tipo String que van a contener la latitud y longitud del sensor GPS.
    var latitude : String?
    var longitude : String?
	
    //Action del boton guardar.
	@IBAction func guardarAction(_ sender: Any)
	{
        //Hora actual UNIX.
        let timeInterval = NSDate().timeIntervalSince1970
        //Hora actual UNIX en String para concatener.
        let timeString = String(Int(timeInterval))
        //Si la ubicacion en base al sensorGPS esta activado.
        if(self.ubicacionSwitch.isOn)
        {
            //Armo el mensaje para pasar al SE.
            var mensaje = "5;"+self.ssidText.text!+";"+self.passwordText.text!
            mensaje+=";"
            mensaje+=self.latitude!
            mensaje+=";"
            mensaje+=self.longitude!
            mensaje+=";"
            mensaje+=timeString
            let data: Data = mensaje.data(using: String.Encoding.utf8)!
            FirstViewController.esp32Shared.writeValue(data, for: Array(FirstViewController.characteristicsShared)[0].value, type: CBCharacteristicWriteType.withResponse)
        }
        //Si la ubicacion en base al sensorGPS no esta activado.
        else
        {
            var mensaje = "3;"+self.ssidText.text!+";"+self.passwordText.text!
            mensaje+=";"
            mensaje+=self.ubicacionText.text!
            mensaje+=";"
            mensaje+=timeString
            let data: Data = mensaje.data(using: String.Encoding.utf8)!
            FirstViewController.esp32Shared.writeValue(data, for: Array(FirstViewController.characteristicsShared)[0].value, type: CBCharacteristicWriteType.withResponse)
        }
		
	}
    
    //Switch de ubicacion.
    @IBAction func switchAction(_ sender: Any)
    {
        if(self.ubicacionSwitch.isOn)
        {
            self.ubicacionText.isEnabled = false
        }
        else
        {
            self.ubicacionText.isEnabled = true
            self.ubicacionText.text = ""
        }
    }
    
    
	override func viewDidLoad()
	{
		// Adaptación de la pantalla al mostrar ú ocultar el teclado.
		NotificationCenter.default.addObserver(self, selector: #selector(keyboardWillShow), name: UIResponder.keyboardWillShowNotification, object: nil)
		NotificationCenter.default.addObserver(self, selector: #selector(keyboardWillHide), name: UIResponder.keyboardWillHideNotification, object: nil)
		super.viewDidLoad()
		
		self.setupHideKeyboardOnTap()
        
        //Por defecto el input de ubicacion empieza desactivado.
        self.ubicacionText.isEnabled = false
        
        //Mensaje al usuario para permitir la activacion del sensor GPS.
        self.locationManager.requestWhenInUseAuthorization()
        
        //Activacion del sensor GPS.
        if CLLocationManager.locationServicesEnabled() {
            
            locationManager.delegate = self
            locationManager.desiredAccuracy = kCLLocationAccuracyNearestTenMeters
            locationManager.startUpdatingLocation()
        }
    }
    
    // Obtención de coordenadas en base al sensor GPS.
    func locationManager(_ manager: CLLocationManager, didUpdateLocations locations: [CLLocation]) {
        
        let locValue:CLLocationCoordinate2D = manager.location!.coordinate
        print("locations = \(locValue.latitude) \(locValue.longitude)")
        self.latitude = String(locValue.latitude)
        self.longitude = String(locValue.longitude)
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

