package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Patreon;

public class PatreonSteps {
	Patreon patreon= new Patreon(WebDriverFactory.get());

	@When("I login Patreon with '(.*)'")
	public void login(String username){
		patreon.goToLogin();
		patreon.enterEmail(username);
		String password =System.getenv().get("PATREONPASS");
		patreon.enterPassword(password);
		patreon.submit();
		
	}
	@When("I go to Patreon login page")
	public void pressLogin(){
		patreon.goToLogin();
		Assert.assertTrue("Expected to be at login page", patreon.checkAtLoginPage());
		
	}
	@Then("I should be logged in Patreon")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",patreon.checkLogin());
	}
	
	@When("I logout Patreon")
	public void pressLogout(){
		patreon.goTodDashboard();
		patreon.logout();
	}
}
