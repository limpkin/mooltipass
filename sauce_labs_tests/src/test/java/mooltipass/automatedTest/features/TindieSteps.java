package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Tindie;

public class TindieSteps {
	Tindie tindie = new Tindie(WebDriverFactory.get());
	
	@When("I login tindie with '(.*)'")
	public void login(String username){
		tindie.goToLogin();
		tindie.enterEmail(username);
		String password =System.getenv().get("TINDIEPASS");
		tindie.enterPassword(password);
		tindie.submit();
		
	}

	@Then("I should be logged in tindie")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",tindie.checkLogin());
	}
	
	@When("I go to tindie login page")
	public void pressLogin(){
		tindie.goToLogin();
		Assert.assertTrue("Expected to be at login page", tindie.checkAtLoginPage());
		
	}
	
	@When("I logout tindie")
	public void pressLogout(){
		tindie.logout();
	}
}
