package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Kenwoodworld;

public class KenwoodworldSteps {
	Kenwoodworld kenwoodworld = new Kenwoodworld(WebDriverFactory.get());
	
	@When("I login kenwoodworld with '(.*)'")
	public void login(String username){
		kenwoodworld.goToLogin();
		kenwoodworld.enterEmail(username);
		String password =System.getenv().get("KENPASS");
		kenwoodworld.enterPassword(password);
		kenwoodworld.submit();
		
	}

	@Then("I should be logged in kenwoodworld")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",kenwoodworld.checkLogin());
	}
	
	@When("I go to kenwoodworld login page")
	public void pressLogin(){
		kenwoodworld.goToLogin();
		Assert.assertTrue("Expected to be at login page", kenwoodworld.checkAtLoginPage());
		
	}
	
	@When("I logout kenwoodworld")
	public void pressLogout(){
		kenwoodworld.logout();
	}
}