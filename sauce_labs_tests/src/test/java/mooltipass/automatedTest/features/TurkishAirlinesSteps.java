package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.TurkishAirlines;

public class TurkishAirlinesSteps {

	TurkishAirlines turkishAirlines= new TurkishAirlines(WebDriverFactory.get());
	
	@When("I login turkish airlines with '(.*)'")
	public void login(String username){
		turkishAirlines.goToLogin();
		turkishAirlines.enterEmail(username);
		String password =System.getenv().get("TURKISHPASS");
		turkishAirlines.enterPassword(password);
		turkishAirlines.submit();
		
	}
	@When("I logout turkish airlines")
	public void pressLogout(){
		turkishAirlines.logout();
		
	}
	@When("I go to turkish airlines login page")
	public void pressLogin(){
		turkishAirlines.goToLogin();
		Assert.assertTrue("Expected to be at login page", turkishAirlines.checkAtLoginPage());
		
	}
	
	@Then("I should be logged in turkish airlines")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",turkishAirlines.checkLogin());
	}
}
