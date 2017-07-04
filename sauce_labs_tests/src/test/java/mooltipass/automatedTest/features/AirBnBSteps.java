package mooltipass.automatedTest.features;

import org.openqa.selenium.support.ui.Sleeper;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.AirBnB;
import mooltipass.automatedTest.pageObjects.Microspot;
public class AirBnBSteps {

	
	AirBnB airbnb= new AirBnB(WebDriverFactory.get());

	@When("I login airBnB with '(.*)'")
	public void login(String username){
		airbnb.goToLogin();
		airbnb.enterEmail(username);
		String password = System.getenv().get("AIRBNBPASS");
		airbnb.enterPassword(password);
		airbnb.submit();
		
	}
	@When("I go to airBnB login page")
	public void pressLogin(){
		airbnb.goToLogin();
		Assert.assertTrue("Expected to be at login page", airbnb.checkAtLoginPage());
		
	}
	@Then("I should be logged in airBnB")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",airbnb.checkLogin());
	}
	
	@When("I logout airBnB")
	public void pressLogout(){
		airbnb.goTodDashboard();
		airbnb.logout();
		airbnb.approveLogout();
	}
	
}
