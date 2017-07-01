package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Ricardo;

public class RicardoSteps {
	Ricardo ricardo = new Ricardo(WebDriverFactory.get());
	
	@When("I login ricardo with '(.*)'")
	public void login(String username){
		ricardo.goToLogin();
		ricardo.enterEmail(username);
		String password =System.getenv().get("RICARDOPASS");
		ricardo.enterPassword(password);
		ricardo.submit();
		
	}

	@Then("I should be logged in ricardo")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",ricardo.checkLogin());
	}
	
	@When("I go to ricardo login page")
	public void pressLogin(){
		ricardo.goToLogin();
		Assert.assertTrue("Expected to be at login page", ricardo.checkAtLoginPage());
		
	}
	
	@When("I logout ricardo")
	public void pressLogout(){
		ricardo.logout();
	}
}
