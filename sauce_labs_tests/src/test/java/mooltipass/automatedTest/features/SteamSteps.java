package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Steam;

public class SteamSteps {
	Steam steam= new Steam(WebDriverFactory.get());
	

	@When("I login Steam with '(.*)'")
	public void login(String email){
		steam.goToLogin();
		steam.enterEmail(email);
		String password =System.getenv().get("STEAMPASS"); 
		steam.enterPassword(password);
		steam.submit();
	}
	
	@When("I go to Steam login page")
	public void goTologin(){
		steam.goToLogin();
		Assert.assertTrue("Expected User to be at login Page",steam.checkAtLoginPage());
	}
	@Then("I should be logged in Steam")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",steam.checkLogin());
	}
	
	@When("I logout Steam")
	public void pressLogout(){
		steam.goTodDashboard();
		steam.logout();
	}
}