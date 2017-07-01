package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.esp8266;;

public class esp8266Steps {
	 esp8266 esp8266= new esp8266(WebDriverFactory.get());

		@When("I login esp8266 with '(.*)'")
		public void login(String username){
			esp8266.goToLogin();
			esp8266.enterEmail(username);
			String password =System.getenv().get("ESPPASS");
			esp8266.enterPassword(password);
			esp8266.submit();
			
		}
		@When("I go to esp8266 login page")
		public void pressLogin(){
			esp8266.goToLogin();
			
		}
		@Then("I should be logged in esp8266")
		public void checkLogin(){
			Assert.assertTrue("Expected User to be logged in",esp8266.checkLogin());
		}
		
		@When("I logout esp8266")
		public void pressLogout(){
			esp8266.goTodDashboard();
			esp8266.logout();
		}
}
