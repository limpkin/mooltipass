package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Komoot;

public class KomootSteps {

	Komoot komoot = new Komoot(WebDriverFactory.get());
	
	@When("I login komoot with '(.*)'")
	public void login(String username){
		komoot.goToLogin();
		komoot.enterEmail(username);
		String password =System.getenv().get("KOMOOTPASS");
		komoot.enterPassword(password);
		komoot.submit();
		
	}

	@Then("I should be logged in komoot")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",komoot.checkLogin());
	}
	
	@When("I go to komoot login page")
	public void pressLogin(){
		komoot.goToLogin();
		Assert.assertTrue("Expected to be at login page", komoot.checkAtLoginPage());
		
	}
	
	@When("I logout komoot")
	public void pressLogout(){
		komoot.logout();
	}
}
