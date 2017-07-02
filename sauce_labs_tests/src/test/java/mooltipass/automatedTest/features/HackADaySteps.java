package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.HackADay;

public class HackADaySteps {
	HackADay hAD= new HackADay(WebDriverFactory.get());

	@When("I login HackADay with '(.*)'")
	public void login(String username){
		hAD.goToLogin();
		hAD.enterEmail(username);
		String password =System.getenv().get("HACKPASS");
		hAD.enterPassword(password);
		hAD.submit();
		
	}
	@When("I go to HackADay login page")
	public void pressLogin(){
		hAD.goToLogin();
		//Assert.assertTrue("Expected to be at login page", hAD.checkAtLoginPage());
		
	}
	@Then("I should be logged in HackADay")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",hAD.checkLogin());
	}
	
	@When("I logout HackADay")
	public void pressLogout(){
		hAD.goTodDashboard();
		hAD.logout();
	}
}
