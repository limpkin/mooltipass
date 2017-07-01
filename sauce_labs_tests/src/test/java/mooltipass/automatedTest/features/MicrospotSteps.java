package mooltipass.automatedTest.features;

import org.openqa.selenium.support.ui.Sleeper;

import cucumber.api.java.en.Given;
import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Microspot;
import mooltipass.automatedTest.pageObjects.Techmania;;

public class MicrospotSteps {

	Microspot microspot= new Microspot(WebDriverFactory.get());
	
	
	@When("I login Microspot with '(.*)'")
	public void login(String username){
		microspot.goToLogin();
		microspot.enterEmail(username);
		String password = System.getenv().get("MICROPASS");
		microspot.enterPassword(password);
		microspot.submit();
		
	}
	@When("I go to Microspot login page")
	public void pressLogin(){
		microspot.goToLogin();
		Assert.assertTrue("Expected to be at login page", microspot.checkAtLoginPage());
		
	}
	@Then("I should be logged in Microspot")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",microspot.checkLogin());
	}
	
	@When("I logout Microspot")
	public void pressLogout(){
		microspot.logout();
	}
	

}
