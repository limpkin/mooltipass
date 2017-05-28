package mooltipass.automatedTest.config;

import org.apache.commons.configuration.CompositeConfiguration;
import org.apache.commons.configuration.Configuration;
import org.apache.commons.configuration.ConfigurationException;
import org.apache.commons.configuration.EnvironmentConfiguration;
import org.apache.commons.configuration.PropertiesConfiguration;
import org.apache.commons.configuration.SystemConfiguration;

public class ConfigFactory {
	private static CompositeConfiguration config = null;
	
	public static Configuration get(){
		if(config == null){
			config = new CompositeConfiguration();
			config.addConfiguration(new SystemConfiguration());
			config.addConfiguration(new EnvironmentConfiguration());
			try {
				config.addConfiguration(new PropertiesConfiguration("conf.properties"));
			} catch (ConfigurationException e) {
				e.printStackTrace();
			}		
		}
		return config;
	}
	
}
