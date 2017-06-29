package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.DropBox;

public class DropBoxSteps {

	DropBox dropBox= new DropBox(WebDriverFactory.get());

	@When("I login DropBox with '(.*)' and '(.*)'")
	public void login(String username,String password){
		dropBox.goToLogin();
		dropBox.enterEmail(username);
		dropBox.enterPassword(password);
		dropBox.submit();
		
	}
	@When("I go to DropBox login page")
	public void pressLogin(){
		dropBox.goToLogin();
		Assert.assertTrue("Expected to be at login page", dropBox.checkAtLoginPage());
		
	}
	@Then("I should be logged in DropBox")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",dropBox.checkLogin());
	}
	
	@When("I logout DropBox")
	public void pressLogout(){
		dropBox.goTodDashboard();
		dropBox.logout();
	}
}
